#include <stdio.h>
#include <stdlib.h>
#include "tipos.h"
#include <string.h>
#include "funciones.h"
#include <time.h>
#include <stdint.h>
#include "imagen.h"
#include "Registros.h"

void disassembler(MaquinaVirtual *vm);

static int32_t tieneExtension(const char *nombre, const char *extension){
    size_t largoNombre, largoExtension;

    largoNombre = strlen(nombre);
    largoExtension = strlen(extension);
    return largoNombre > largoExtension && strcmp(nombre + largoNombre - largoExtension, extension) == 0;
}

/* Lee 2 bytes del archivo en big-endian (el primero es el mas significativo).
   Son dos lineas separadas a proposito: en (fgetc(f) << 8) | fgetc(f)
   C no garantiza cual fgetc se ejecuta primero. */
static int32_t leer16(FILE *f, uint32_t *valor){
    int alto = fgetc(f);
    int bajo = fgetc(f);
    if (alto == EOF || bajo == EOF)
        return -1;
    *valor = ((uint32_t)alto << 8) | (uint32_t)bajo;
    return 0;
}

/* Lee la cabecera del .vmx en el mismo orden en que esta en el archivo.
   Version 1: solo el tamano del codigo. Version 2: los 5 tamanos y el entry point.
   Lo que no viene en la cabecera queda en 0. */
static int32_t leerCabecera(FILE *f, Cabecera *cab){
    char id[MAX_IDENTIFICADOR];
    int32_t error;

    memset(cab, 0, sizeof(*cab));
    if (fread(id, 1, MAX_IDENTIFICADOR, f) != MAX_IDENTIFICADOR || memcmp(id, "VMX26", MAX_IDENTIFICADOR) != 0){
        printf("No se puede ejecutar el programa\n");
        return -1;
    }
    cab->version = (uint8_t)fgetc(f);

    if (cab->version == 1)
        error = leer16(f, &cab->tamanos[SEG_CODE]);
    else if (cab->version == 2)               // mismo orden que la tabla del enunciado
        error = leer16(f, &cab->tamanos[SEG_CODE])
             || leer16(f, &cab->tamanos[SEG_DATA])
             || leer16(f, &cab->tamanos[SEG_EXTRA])
             || leer16(f, &cab->tamanos[SEG_STACK])
             || leer16(f, &cab->tamanos[SEG_CONST])
             || leer16(f, &cab->entryPoint);
    else {
        printf("No se puede ejecutar el programa\n");
        return -1;
    }
    if (error){
        printf("Error: cabecera incompleta\n");
        return -1;
    }
    return 0;
}

/* Copia 'tamano' bytes del archivo al comienzo del segmento apuntado por el
   registro 'reg' (CS, KS, ...). La base fisica se busca en la tabla a traves
   del registro, nunca con un numero de entrada fijo. */
static int32_t copiarSegmento(FILE *f, MaquinaVirtual *vm, int32_t reg, uint32_t tamano){
    uint32_t base;

    if (tamano == 0)
        return 0;
    base = (uint32_t)vm->segmentos[(uint32_t)vm->registros[reg] >> 16] >> 16;
    return fread(&vm->memoria.datos[base], 1, tamano, f) == tamano ? 0 : -1;
}

#ifdef DEBUG_CARGA
/* Para controlar la carga: compilar con -DDEBUG_CARGA */
static void imprimirEstado(MaquinaVirtual *vm, Cabecera *cab){
    static const char *nombres[] = { "CS", "DS", "ES", "SS", "KS", "PS" };
    int32_t i;

    printf("v%u code=%u data=%u extra=%u stack=%u const=%u entry=%u\n",
           cab->version, cab->tamanos[SEG_CODE], cab->tamanos[SEG_DATA], cab->tamanos[SEG_EXTRA],
           cab->tamanos[SEG_STACK], cab->tamanos[SEG_CONST], cab->entryPoint);
    for (i = 0; i < CANT_SEGMENTOS; i++)
        printf("tabla[%d] = %08X\n", i, (uint32_t)vm->segmentos[i]);
    for (i = 0; i < 6; i++)
        printf("%s=%08X ", nombres[i], (uint32_t)vm->registros[REG_CS + i]);
    printf("IP=%08X SP=%08X\n", (uint32_t)vm->registros[REG_IP], (uint32_t)vm->registros[REG_SP]);
}
#endif

static int32_t cargarVmx(MaquinaVirtual *vm, const char *nombreArchivo, uint32_t kib){
    FILE *file;
    Cabecera cabecera;

    file = fopen(nombreArchivo, "rb");
    if (file == NULL){
        printf("Error al abrir archivo\n");
        return -1;
    }
    if (leerCabecera(file, &cabecera) == -1){
        fclose(file);
        return -1;
    }
    if (crearMemoria(&vm->memoria, kib) == -1){
        printf("Error: no se pudo reservar la memoria.\n");
        fclose(file);
        return -1;
    }
    // Version 1: no hay tamanos de segmentos en la cabecera; como en la primera
    // parte, el Data Segment ocupa toda la memoria que sobra despues del codigo.
    // (Un segmento mide como maximo 0xFFFF bytes: es lo que entra en la tabla.)
    if (cabecera.version == 1){
        if (cabecera.tamanos[SEG_CODE] > vm->memoria.tamano){
            printf("Error: memoria insuficiente\n");
            liberarMemoria(&vm->memoria);
            fclose(file);
            return -1;
        }
        cabecera.tamanos[SEG_DATA] = vm->memoria.tamano - cabecera.tamanos[SEG_CODE];
        if (cabecera.tamanos[SEG_DATA] > 0xFFFF)
            cabecera.tamanos[SEG_DATA] = 0xFFFF;
    }

    // Primero se ubican los segmentos (tabla y registros) y recien despues se
    // copia el contenido, porque hasta no armar la tabla no se sabe donde va cada uno.
    if (iniciaMaquinaVirtual(vm, cabecera) == -1){
        liberarMemoria(&vm->memoria);
        fclose(file);
        return -1;
    }

    // En el .vmx, despues de la cabecera viene el codigo y a continuacion las constantes
    if (copiarSegmento(file, vm, REG_CS, cabecera.tamanos[SEG_CODE]) == -1 ||
        copiarSegmento(file, vm, REG_KS, cabecera.tamanos[SEG_CONST]) == -1){
        printf("Error: archivo VMX incompleto\n");
        liberarMemoria(&vm->memoria);
        fclose(file);
        return -1;
    }
    fclose(file);
#ifdef DEBUG_CARGA
    imprimirEstado(vm, &cabecera);
#endif
    return 0;
}

int main (int argc, char *argv[]){
    if (argc < 2)
        printf("No se pasaron los parametros esperados\n");
    else{
        MaquinaVirtual vm = {0};
        const char *archivoVmx = NULL;
        int32_t i, mostrarDisassembler, resultado, kib;

        mostrarDisassembler = 0;
        kib = MEMORIA_KIB_DEFECTO;
        for (i = 1; i < argc; i++){
            if (strcmp(argv[i], "-d") == 0)
                mostrarDisassembler = 1;
            else if (strncmp(argv[i], "m=", 2) == 0){     // m=M: tamano de la memoria en KiB
                kib = atoi(argv[i] + 2);
                if (kib <= 0){
                    printf("Tamano de memoria invalido: %s\n", argv[i]);
                    return 1;
                }
            }
            else if (tieneExtension(argv[i], ".vmi"))
                vm.archivoImagen = argv[i];
            else if (tieneExtension(argv[i], ".vmx"))
                archivoVmx = argv[i];
        }

        if (archivoVmx == NULL && vm.archivoImagen == NULL){
            printf("Se requiere al menos un archivo .vmx o .vmi\n");
            return 1;
        }

        if (archivoVmx != NULL)
            resultado = cargarVmx(&vm, archivoVmx, (uint32_t)kib);
        else
            resultado = cargarImagen(&vm, vm.archivoImagen);      // sin .vmx se continua la ejecucion desde la imagen
        if (resultado == -1)
            return 1;

        if (mostrarDisassembler)
            disassembler(&vm);
        vm.corriendo = 1;
        srand(time(NULL)); //Para correcta funcionalidad del RND
        procesaPrograma(&vm);
        liberarMemoria(&vm.memoria);
    }
    return 0;
}

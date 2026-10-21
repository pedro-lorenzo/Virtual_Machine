#include <stdio.h>
#include <stdlib.h>
#include "tipos.h"
#include <string.h>
#include "funciones.h"
#include <time.h>
#include <stdint.h>
#include "imagen.h"

void disassembler(MaquinaVirtual *vm);

static int32_t tieneExtension(const char *nombre, const char *extension){
    size_t largoNombre, largoExtension;

    largoNombre = strlen(nombre);
    largoExtension = strlen(extension);
    return largoNombre > largoExtension && strcmp(nombre + largoNombre - largoExtension, extension) == 0;
}

static int32_t cargarVmx(MaquinaVirtual *vm, const char *nombreArchivo){
    FILE *file;
    Cabecera cabecera;
    int32_t i;

    file = fopen(nombreArchivo, "rb");
    if (file == NULL){
        printf("Error al abrir archivo\n");
        return -1;
    }
    if (fread(&cabecera, 1, sizeof(cabecera), file) != sizeof(cabecera)){
        printf("Error: cabecera incompleta\n");
        fclose(file);
        return -1;
    }
    cabecera.tamano = (cabecera.tamano << 8) | (cabecera.tamano >> 8);
    if (crearMemoria(&vm->memoria, MEMORIA_KIB_DEFECTO) == -1){
        printf("Error: no se pudo reservar la memoria.\n");
        fclose(file);
        return -1;
    }
    if (cabecera.tamano > vm->memoria.tamano){
        printf("Error: el programa no entra en la memoria.\n");
        fclose(file);
        return -1;
    }
    if (memcmp(cabecera.identificador, "VMX26", 5) == 0 && cabecera.version == 1){       // memcmp compara exactamente los 5 bytes, ya que identificador no es una cadena
        for (i = 0; i < cabecera.tamano; i++){
            if (!fread(&vm->memoria.datos[i], 1, sizeof(uint8_t), file)){
                printf("Error: archivo VMX incompleto\n");
                fclose(file);
                return -1;
            }
        }
        fclose(file);
        iniciaMaquinaVirtual(cabecera, vm->segmentos, vm->registros, vm->memoria.tamano);
        return 0;
    }
    printf("No se puede ejecutar el programa\n");
    fclose(file);
    return -1;
}

int main (int argc, char *argv[]){
    if (argc < 2 || argc > 4)
        printf("No se pasaron los parametros esperados\n");
    else{
        MaquinaVirtual vm = {0};
        const char *archivoVmx = NULL;
        int32_t i, mostrarDisassembler, resultado;

        mostrarDisassembler = 0;
        for (i = 1; i < argc; i++){
            if (strcmp(argv[i], "-d") == 0)
                mostrarDisassembler = 1;
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
            resultado = cargarVmx(&vm, archivoVmx);
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

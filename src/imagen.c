#include <stdio.h>
#include <string.h>
#include "imagen.h"
#include "funciones.h"

static int32_t escribirEntero32(FILE *archivo, int32_t valor){
    uint8_t bytes[4];
    uint32_t sinSigno;

    sinSigno = (uint32_t)valor;
    bytes[0] = (uint8_t)(sinSigno >> 24);
    bytes[1] = (uint8_t)(sinSigno >> 16);
    bytes[2] = (uint8_t)(sinSigno >> 8);
    bytes[3] = (uint8_t)sinSigno;
    return fwrite(bytes, 1, 4, archivo) == 4 ? 0 : -1;
}

static int32_t leerEntero32(FILE *archivo, int32_t *valor){
    uint8_t bytes[4];

    if (fread(bytes, 1, 4, archivo) != 4)
        return -1;
    *valor = (int32_t)(((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | ((uint32_t)bytes[2] << 8) | bytes[3]);
    return 0;
}

int32_t guardarImagen(MaquinaVirtual *vm, const char *nombreArchivo){
    FILE *archivo;
    uint8_t cabecera[8] = {'V', 'M', 'I', '2', '6', 1, 0, 0};
    uint32_t kib;
    int32_t i;
    int32_t error;

    archivo = fopen(nombreArchivo, "wb");
    if (archivo == NULL)
        return -1;

    kib = vm->memoria.tamano / 1024;
    cabecera[6] = (uint8_t)(kib >> 8);
    cabecera[7] = (uint8_t)kib;

    error = fwrite(cabecera, 1, 8, archivo) != 8;
    for (i = 0; i < CANT_REGISTROS && !error; i++)
        error = escribirEntero32(archivo, vm->registros[i]) == -1;
    for (i = 0; i < CANT_SEGMENTOS && !error; i++)
        error = escribirEntero32(archivo, vm->segmentos[i]) == -1;
    if (!error)
        error = fwrite(vm->memoria.datos, 1, vm->memoria.tamano, archivo) != vm->memoria.tamano;

    fclose(archivo);
    return error ? -1 : 0;
}

int32_t cargarImagen(MaquinaVirtual *vm, const char *nombreArchivo){
    FILE *archivo;
    uint8_t cabecera[8];
    uint32_t kib;
    int32_t i;
    int32_t error;

    archivo = fopen(nombreArchivo, "rb");
    if (archivo == NULL){
        printf("Error al abrir archivo\n");
        return -1;
    }
    if (fread(cabecera, 1, 8, archivo) != 8 || memcmp(cabecera, "VMI26", 5) != 0 || cabecera[5] != 1){
        printf("No se puede ejecutar el programa\n");
        fclose(archivo);
        return -1;
    }

    kib = ((uint32_t)cabecera[6] << 8) | cabecera[7];
    if (kib == 0 || crearMemoria(&vm->memoria, kib) == -1){
        printf("Error: no se pudo reservar la memoria.\n");
        fclose(archivo);
        return -1;
    }

    error = 0;
    for (i = 0; i < CANT_REGISTROS && !error; i++)
        error = leerEntero32(archivo, &vm->registros[i]) == -1;
    for (i = 0; i < CANT_SEGMENTOS && !error; i++)
        error = leerEntero32(archivo, &vm->segmentos[i]) == -1;
    if (!error)
        error = fread(vm->memoria.datos, 1, vm->memoria.tamano, archivo) != vm->memoria.tamano;

    fclose(archivo);
    if (error){
        printf("Error: archivo VMI incompleto\n");
        liberarMemoria(&vm->memoria);
        return -1;
    }
    return 0;
}

void breakpoint(MaquinaVirtual *vm){
    char linea[64];
    int32_t accion;

    if (vm->archivoImagen == NULL)
        return;

    vm->breakpointEjecutado = 1;
    if (guardarImagen(vm, vm->archivoImagen) == -1)
        printf("Error: no se pudo guardar la imagen\n");

    accion = 0;
    while (!accion){
        if (fgets(linea, sizeof(linea), stdin) == NULL)
            accion = 'q';
        else if (linea[0] == 'g' || linea[0] == 'q' || linea[0] == '\n')
            accion = linea[0];
    }

    if (accion == 'g')
        vm->pasoAPaso = 0;
    else if (accion == 'q')
        vm->corriendo = 0;
    else
        vm->pasoAPaso = 1;
}
#ifndef TIPOS_H
#define TIPOS_H

#include <stdint.h>

#define CANT_REGISTROS 32
#define CANT_SEGMENTOS 8      // entradas de la TABLA de descriptores (siempre 8)
enum {SEG_PARAM, SEG_CONST, SEG_CODE, SEG_DATA, SEG_EXTRA, SEG_STACK };
#define MAX_IDENTIFICADOR 5
#define MEMORIA_KIB_DEFECTO 16

#define CANT_SEG_PROCESO 6   // segmentos del proceso (SEG_PARAM .. SEG_STACK)


typedef struct {
    uint8_t *datos;
    uint32_t tamano;
} Memoria;

/* Es signed porque los segmentos no utilizados se representan con -1. */
typedef int32_t tSegmento[CANT_SEGMENTOS];

typedef int32_t tRegistro[CANT_REGISTROS];

typedef struct {
    Memoria memoria;
    tSegmento segmentos;
    tRegistro registros;
    const char *archivoImagen;   // NULL si no se paso .vmi
    int32_t pasoAPaso;           // 1 = frenar tras cada instruccion
    int32_t breakpointEjecutado; // 1 si la instruccion en curso fue SYS F
    int corriendo; // 1 = sigue ejecutando, 0 = terminó (STOP o error)
} MaquinaVirtual;

typedef struct {
    uint32_t entryPoint;          // uint32_t porque lo carga leer16
    uint8_t version;
    uint32_t tamanos[CANT_SEG_PROCESO];   // indexado con SEG_*
} Cabecera;
#endif

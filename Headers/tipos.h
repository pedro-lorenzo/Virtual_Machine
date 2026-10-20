#ifndef TIPOS_H
#define TIPOS_H

#include <stdint.h>

#define CANT_REGISTROS 32
#define CANT_SEGMENTOS 8
#define MAX_IDENTIFICADOR 5
#define MEMORIA_KIB_DEFECTO 16


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
    int corriendo; // 1 = sigue ejecutando, 0 = terminó (STOP o error)
} MaquinaVirtual;

typedef struct {
    int8_t identificador[MAX_IDENTIFICADOR];
    uint8_t version;
    uint16_t tamano;
} Cabecera;
#endif

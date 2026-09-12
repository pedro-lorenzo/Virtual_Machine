#ifndef TIPOS_H
#define TIPOS_H

#define TAM_MEMORIA 16384
#define CANT_REGISTROS 32
#define CANT_SEGMENTOS 8
#define MAX_IDENTIFICADOR 5

typedef struct {
    unsigned char datos[TAM_MEMORIA];
} Memoria;

/* Es signed porque los segmentos no utilizados se representan con -1. */
typedef int tSegmento[CANT_SEGMENTOS];

typedef unsigned int tRegistro[CANT_REGISTROS];

typedef struct {
    Memoria memoria;
    tSegmento segmentos;
    tRegistro registros;
    int corriendo; // 1 = sigue ejecutando, 0 = terminó (STOP o error)
} MaquinaVirtual;

typedef struct {
    char identificador[MAX_IDENTIFICADOR];
    unsigned char version;
    unsigned short tamano;
} Cabecera;

#endif

#define TAM_MEMORIA 16384
#define CANT_REGISTROS 32
#define CANT_SEGMENTOS 8
#define MAX_IDENTIFICADOR 5

typedef struct {
    char datos[TAM_MEMORIA];
} Memoria;

typedef int tSegmento[CANT_SEGMENTOS];

typedef struct {
    int  registros[CANT_REGISTROS]; // 4 bytes cada uno
} TablaRegistros;


typedef struct {
    Memoria memoria;
    tSegmento tabla_segmentos;
    TablaRegistros registros;
    int corriendo; // 1 = sigue ejecutando, 0 = terminó (STOP o error)
} MaquinaVirtual;

typedef struct{
    char identificador[MAX_IDENTIFICADOR];
    char version;
    short tamano;
}Cabecera;
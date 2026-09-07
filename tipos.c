#define TAM_MEMORIA 16384
#define CANT_REGISTROS 32
#define CANT_SEGMENTOS 8
#define MAX_IDENTIFICADOR 5

typedef struct {
    char datos[TAM_MEMORIA];
} Memoria;

typedef struct {
    short base;   // 2 bytes
    short tamano; // 2 bytes
} DescriptorSegmento;

typedef struct {
    DescriptorSegmento segmentos[CANT_SEGMENTOS];
} TablaSegmentos;


typedef struct {
    int  registros[CANT_REGISTROS]; // 4 bytes cada uno
} Registro;

typedef struct {
    Memoria memoria;
    TablaSegmentos tabla_segmentos;
    TablaRegistros registros;
    int corriendo; // 1 = sigue ejecutando, 0 = terminó (STOP o error)
} MaquinaVirtual;

typedef struct{
    char identificador[MAX_IDENTIFICADOR];
    char version;
    short tamano;
}Cabecera;
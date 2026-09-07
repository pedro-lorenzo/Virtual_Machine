#include tipos.h
#include <string.h>

FILE *file;
file = fopen("kwbv", "rb");
Cabecera Cabecera;
Memoria memoria;
TablaSegmentos segmentos;
TablaRegistros registros;

if (file != NULL){
    fread(&Cabecera, 1, sizeof(Cabecera), file);
    if (strcmp(Cabecera.identificador, "VMX26") && Cabecera.version == 1){
        for (int i = 0; i < Cabecera.tamano; i++){
            fread(&memoria.datos[i], 1, sizeof(char), file);
        }
        segmentos[0].base = 0;
        segmentos[0].tamano = Cabecera.tamano;
        segmentos[1].base = Cabecera.tamano;
        segmentos[1].tamano = TAM_MEMORIA - Cabecera.tamano;

        regitros[26] = 0x00000000;
        registros[27] = 0x00010000;
    }
}
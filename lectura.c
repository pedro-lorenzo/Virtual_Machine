#include tipos.h
#include <string.h>
#include "funciones.h"

int main (){
    FILE *file;
    file = fopen("kwbv", "rb");
    Cabecera Cabecera;
    Memoria memoria;
    tSegmento segmento;
    TablaRegistros registros;

    if (file != NULL){
        fread(&Cabecera, 1, sizeof(Cabecera), file);
        if (strcmp(Cabecera.identificador, "VMX26") && Cabecera.version == 1){
            for (int i = 0; i < Cabecera.tamano; i++){
                fread(&memoria.datos[i], 1, sizeof(char), file);
            }
            fclose(file);
            iniciaEstructuras(cabecera, segmento, registros);
        }
    }

    procesaPrograma(memoria, registros, segmento);

    return 0;
}
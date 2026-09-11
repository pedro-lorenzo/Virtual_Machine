#include tipos.h
#include <string.h>
#include "funciones.h"

int main (){
    FILE *file;
    file = fopen("kwbv", "rb");
    Cabecera Cabecera;
    MaquinaVirtual vm;

    if (file != NULL){
        fread(&Cabecera, 1, sizeof(Cabecera), file);
        if (strcmp(Cabecera.identificador, "VMX26") && Cabecera.version == 1){
            for (int i = 0; i < Cabecera.tamano; i++){
                fread(&vm.memoria.datos[i], 1, sizeof(char), file);
            }
            fclose(file);
            iniciaMaquinaVirtual(Cabecera,vm.segmentos,&vm.registros);
        }
    }

    procesaPrograma(&vm);

    return 0;
}
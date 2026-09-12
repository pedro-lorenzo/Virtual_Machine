#include <stdio.h>
#include "tipos.h"
#include <string.h>
#include "funciones.h"

int main (){
    FILE *file;
    file = fopen("kwbv", "rb");
    Cabecera Cabecera;
    MaquinaVirtual vm;

    if (file != NULL){
        fread(&Cabecera, 1, sizeof(Cabecera), file);
        if (memcmp(cabecera.identificador, "VMX26", 5) == 0 && cabecera.version == 1){       // memcmp compara exactamente los 5 bytes, ya que identificador no es una cadena
            for (int i = 0; i < Cabecera.tamano; i++){
                fread(&vm.memoria.datos[i], 1, sizeof(char), file);
            }
            fclose(file);
            iniciaMaquinaVirtual(Cabecera,vm.segmentos,&vm.registros);
        }
        procesaPrograma(&vm);
    }

    return 0;
}
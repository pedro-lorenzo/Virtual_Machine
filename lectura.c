#include <stdio.h>
#include "tipos.h"
#include <string.h>
#include "funciones.h"

int main (){
    FILE *file;
    file = fopen("kwbv", "rb");
    Cabecera cabecera;
    MaquinaVirtual vm;

    if (file != NULL){
        fread(&cabecera, 1, sizeof(cabecera), file);
        if (memcmp(cabecera.identificador, "VMX26", 5) == 0 && cabecera.version == 1){       // memcmp compara exactamente los 5 bytes, ya que identificador no es una cadena
            for (int i = 0; i < cabecera.tamano; i++){
                fread(&vm.memoria.datos[i], 1, sizeof(char), file);
            }
            fclose(file);
            iniciaMaquinaVirtual(cabecera,vm.segmentos,vm.registros);
            vm.corriendo = 1;
            procesaPrograma(&vm);
        }
        
    }

    return 0;
}
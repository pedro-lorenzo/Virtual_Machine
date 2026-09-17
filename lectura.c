#include <stdio.h>
#include "tipos.h"
#include <string.h>
#include "funciones.h"

int main (int argc, char *argv[]){
    if (argc != 2)
        printf("No se pasaron los parametros esperados\n");
    else{
        FILE *file;
        file = fopen(argv[1], "rb");
        Cabecera cabecera;
        MaquinaVirtual vm;

        if (file != NULL){
            fread(&cabecera, 1, sizeof(cabecera), file);
            cabecera.tamano = (cabecera.tamano << 8) | (cabecera.tamano >> 8);
            if (memcmp(cabecera.identificador, "VMX26", 5) == 0 && cabecera.version == 1){       // memcmp compara exactamente los 5 bytes, ya que identificador no es una cadena
                for (int32_t i = 0; i < cabecera.tamano; i++){
                    fread(&vm.memoria.datos[i], 1, sizeof(uint8_t), file);
                }
                fclose(file);
                iniciaMaquinaVirtual(cabecera,vm.segmentos,vm.registros);
                vm.corriendo = 1;
                procesaPrograma(&vm);
            }
        }
        else
            printf("Error al abrir archivo\n");
    }
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include "tipos.h"
#include <string.h>
#include "funciones.h"
#include <time.h>
#include <stdint.h>

void disassembler(MaquinaVirtual *vm);
int main (int argc, char *argv[]){
    if (argc < 2 || argc > 3)
        printf("No se pasaron los parametros esperados\n");
    else{
        FILE *file;
        file = fopen(argv[1], "rb");
        Cabecera cabecera;
        MaquinaVirtual vm = {0};

        if (file != NULL){
            if (fread(&cabecera, 1, sizeof(cabecera), file) != sizeof(cabecera)){
                printf("Error: cabecera incompleta\n");
                fclose(file);
                return 1;
            }
            cabecera.tamano = (cabecera.tamano << 8) | (cabecera.tamano >> 8);
            if (cabecera.tamano > TAM_MEMORIA){
                printf("Error: el programa no entra en la memoria.\n");
                fclose(file);
                return 1;
            }
            if (memcmp(cabecera.identificador, "VMX26", 5) == 0 && cabecera.version == 1){       // memcmp compara exactamente los 5 bytes, ya que identificador no es una cadena
                for (int32_t i = 0; i < cabecera.tamano; i++){
                    if (!fread(&vm.memoria.datos[i], 1, sizeof(uint8_t), file)){
                        printf("Error: archivo VMX incompleto\n");
                        fclose(file);
                        return 1;
                    }
                }
                fclose(file);
                iniciaMaquinaVirtual(cabecera,vm.segmentos,vm.registros);
                if (argc == 3 && strcmp(argv[2], "-d") == 0)
                    disassembler(&vm);
                vm.corriendo = 1;
                srand(time(NULL)); //Para correcta funcionalidad del RND
                procesaPrograma(&vm);
            }
            else{
                printf("No se puede ejecutar el programa\n");
                fclose(file);
                return 1;
            }
        }
        else
            printf("Error al abrir archivo\n");
    }
    return 0;
}

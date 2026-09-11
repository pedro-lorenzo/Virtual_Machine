#include <stdio.h>
#include "tipos.h"
#include <string.h>
#include "funciones.h"

int main (){
    FILE *file;
    file = fopen("kwbv", "rb");
    Cabecera cabecera;
    Memoria memoria;
    tSegmento segmento;
    TablaRegistros registros;

    if (file != NULL){
        fread(&cabecera, 1, sizeof(cabecera), file);
        if (memcmp(cabecera.identificador, "VMX26", 5) == 0 && cabecera.version == 1){       // memcmp compara exactamente los 5 bytes, ya que identificador no es una cadena
            for (int i = 0; i < cabecera.tamano; i++){
                fread(&memoria.datos[i], 1, sizeof(unsigned char), file);
            }
            fclose(file);
            iniciaEstructuras(cabecera, segmento, registros);
        }
        procesaPrograma(memoria, registros, segmento);
    }

    

    return 0;
}
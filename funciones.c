#include "tipos.h"


void iniciaEstructuras(Cabecera cabecera, tSegmento segmentos, TablaRegistros registros){
    int i;

    segmentos[0] = cabecera.tamano;
    segmentos[1] = cabecera.tamano;
    segmentos[1]= segmentos[1] << 16;
    segmentos[1] = segmentos[1] | TAM_MEMORIA - cabecera.tamano;

    for (i=2; i<CANT_SEGMENTOS; i++)
        segmentos[i] = -1;

    regitros[26] = 0x00000000;  //Inicializacion CS
    registros[27] = 0x00010000; //Inicializacion DS
    registros[0]=registros[26]; //Inicializamos IP con CS
}

int cantidadOperandosALeer(int codigo){
    int aux;

    switch codigo:
        0X0F: return 0;
                break;
        else{
            aux= codigo & 0xF0;
            aux >> 4
            return aux + 1;
        }
}
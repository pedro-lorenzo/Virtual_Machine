#include "tipos.h"


void iniciaMaquinaVirtual(Cabecera cabecera, tSegmento segmentos, tRegistro registros){
    int i;

    segmentos[0] = cabecera.tamano;
    segmentos[1] = cabecera.tamano;
    segmentos[1]= segmentos[1] << 16;
    segmentos[1] = segmentos[1] | TAM_MEMORIA - cabecera.tamano;

    for (i=2; i<CANT_SEGMENTOS; i++)
        segmentos[i] = -1;

    registros[26] = 0x00000000;  //Inicializacion CS
    registros[27] = 0x00010000; //Inicializacion DS
    registros[0]=registros[26]; //Inicializamos IP con CS
}


int cantidadOperandosALeer(int codigo) {
    if (codigo == 0x0F) 
        return 0;
    else
        return ((codigo & 0xF0) >> 4) + 1;
}

int direccionLogicaAFisica(unsigned int direccionLogica, tSegmento segmentos) {
    unsigned int codigoSegmento;
    unsigned int desplazamiento;
    int segmento;
    unsigned int base;
    unsigned int tamano;

    codigoSegmento = direccionLogica >> 16;
    desplazamiento = direccionLogica & 0xFFFF;

    if (codigoSegmento >= CANT_SEGMENTOS) {
        return -1;
    }

    segmento = segmentos[codigoSegmento];

    if (segmento == -1) {
        return -1;
    }

    base = ((unsigned int)segmento) >> 16;
    tamano = ((unsigned int)segmento) & 0xFFFF;

    if (desplazamiento >= tamano) {
        return -1;
    }

    return (int)(base + desplazamiento);
}
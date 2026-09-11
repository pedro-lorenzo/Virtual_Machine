#include "tipos.h"
#include <string.h>
#include "funciones.h"
#include <stdlib.h>

void procesaPrograma(Memoria memoria,TablaRegistros registros, tSegmento segmentos){

    char instruccion, *opA;
    int cantOp;

    instruccion=registros[0];

    registros[1]= instruccion && 0x1F;
    cantOp= cantidadOperandosALeer(registros[1]);
    if (!cantOp)
      registros[0]= -1;
    else
        if (cantOp==1)
            tipoOpA = (instruccion & 0b11000000) >> 6;
        else{
            tipoOpB = (instruccion & 0b11000000) >> 6;
            tipoOpA = (instruccion & 0b00110000) >> 6;
        }
    
    
    indiceMemoria=registros[0]+1;
    opB = (char *) malloc(sizeof(char)*tipoOpB);
    for (i=0; i<tipoOpB; i++){
        opB[i]=memoria[indiceMemoria]; //bien?
        indiceMemoria++;
    }
    
    opA = (char *) malloc(sizeof(char)*tipoOpA);
    for (i=0; i<tipoOpA; i++){
        opA[i]=memoria[indiceMemoria]; 
        indiceMemoria++;
    }

    
}
#include "tipos.h"
#include <string.h>
#include "funciones.h"
#include <stdlib.h>

void procesaPrograma( MaquinaVirtual *vm){

    char instruccion, *opA;
    int cantOp,tipoOpA, tipoOpB,indiceMemoria, i;



    while(vm->corriendo){  // procesa mientras corriendo sea 1, cuando encuentra STOP cambia corriendo a 0 y termina la ejecucion
        instruccion=vm->registros[0];

        vm->registros[1]= instruccion & 0x1F;
        cantOp= cantidadOperandosALeer(vm->registros[1]);
        if (!cantOp){             //Cuando encuentra STOP pone corriendo en 0 y termina la ejecucion
            vm->registros[0]= -1;
            vm->corriendo=0;
            
        }
        
        else{
            if (cantOp==1)
                tipoOpA = (instruccion & 0b11000000) >> 6;
            else{
                tipoOpB = (instruccion & 0b11000000) >> 6;
                tipoOpA = (instruccion & 0b00110000) >> 6;
            }
            
            
            indiceMemoria=vm->registros[0]+1;
            opB = (char *) malloc(sizeof(char)*tipoOpB);
            for (i=0; i<tipoOpB; i++){
                opB[i]=vm->memoria[indiceMemoria]; //bien?
                indiceMemoria++;
            }
            
            opA = (char *) malloc(sizeof(char)*tipoOpA);
            for (i=0; i<tipoOpA; i++){
                opA[i]=vm->memoria[indiceMemoria]; 
                indiceMemoria++;
            }
        }
        
    }
}
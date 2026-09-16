#include "tipos.h"
#include <string.h>
#include "funciones.h"
#include "instrucciones.h"
#include <stdlib.h>

void procesaPrograma( MaquinaVirtual *vm){

    char instruccion;
    int cantOp,tipoOpA, tipoOpB,indiceMemoria, i, direccionFisica, opB, opA;

    
//Mismo problema con vm->memoria[indiceMemoria] — memoria es un struct que envuelve datos[TAM_MEMORIA], así que es vm->memoria.datos[indiceMemoria].
    while(vm->corriendo){  // procesa mientras corriendo sea 1, cuando encuentra STOP cambia corriendo a 0 y termina la ejecucion
        direccionFisica=direccionLogicaAFisica(vm->registros[0], vm->segmentos);
        if (direccionFisica==-1){
            vm->registros[0] = -1;
            vm->corriendo = 0;
            continue; //vuelve a chequear el while para salir
        }
        instruccion= vm->memoria.datos[direccionFisica];

        vm->registros[1]= instruccion & 0x1F;
        cantOp= cantidadOperandosALeer(vm->registros[1]);
        if (!cantOp){             //Cuando encuentra STOP pone corriendo en 0 y termina la ejecucion
            tabla_instrucciones[vm->registros[1]](vm);
        }
        
        else{
            if (cantOp==1){
                tipoOpA = (instruccion & 0b11000000) >> 6;
                tipoOpB = 0;
            }
            else{
                tipoOpB = (instruccion & 0b11000000) >> 6;
                tipoOpA = (instruccion & 0b00110000) >> 4;
            }
            
            
            indiceMemoria=direccionLogicaAFisica(vm->registros[0]+1, vm->segmentos);
            opB=0;
            for (i=0; i<tipoOpB; i++){
                opB = opB << 8;
                opB += vm->memoria.datos[indiceMemoria]; 
                indiceMemoria++;
            }
            
            opA=0;
            for (i=0; i<tipoOpA; i++){
                opA = opA << 8;
                opA += vm->memoria.datos[indiceMemoria]; 
                indiceMemoria++;
            }
        int tamanoInstruccion = 1 + tipoOpA + tipoOpB;   // 1 byte del opcode + los operandos
        vm->registros[0] += tamanoInstruccion;
        vm->registros[2] = (tipoOpA << 24) | (opA & 0x00FFFFFF);   // OP1
        vm->registros[3] = (tipoOpB << 24) | (opB & 0x00FFFFFF);   // OP2
        tabla_instrucciones[vm->registros[1]](vm);
        
    }
    }
}
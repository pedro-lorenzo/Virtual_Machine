#include "tipos.h"
#include <string.h>
#include <stdio.h>
#include "funciones.h"
#include "instrucciones.h"
#include <stdlib.h>
#include "Registros.h"

void procesaPrograma( MaquinaVirtual *vm){

    uint8_t instruccion;
    int32_t cantOp,tipoOpA, tipoOpB, indiceMemoria, i, direccionFisica;
    uint32_t opB, opA;

    
    while(vm->corriendo){  // procesa mientras corriendo sea 1, cuando encuentra STOP cambia corriendo a 0 y termina la ejecucion
        direccionFisica=direccionLogicaAFisica(vm->registros[0], vm->segmentos);
        if (direccionFisica==-1 || direccionFisica>=TAM_MEMORIA){    //Condiciones de corte
            printf("Error: IP fuera del segmento de codigo\n");
            vm->registros[0] = -1;
            vm->corriendo = 0;
            continue; //vuelve a chequear el while para salir
        }
        instruccion= vm->memoria.datos[direccionFisica];

        vm->registros[1]= instruccion & 0x1F;
        if (vm->registros[REG_OPC] >= 0x0B && vm->registros[REG_OPC] <= 0x0E){ //verifica si esta dentro de las operaciones invalidas.
            instruccionInvalida(vm);
            continue;
        }
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
            
            uint32_t tamanoInstruccion = 1 + (uint32_t)tipoOpA + (uint32_t)tipoOpB;   // 1 byte del opcode + los operandos
            uint32_t tamanoCodigo = (uint32_t)vm->segmentos[0] & 0xFFFFu;
            uint32_t offsetIP = (uint32_t)vm->registros[REG_IP] & 0xFFFFu;
            if (offsetIP >= tamanoCodigo || tamanoInstruccion > tamanoCodigo - offsetIP) {
                printf("Error: instruccion incompleta\n");
                vm->corriendo = 0;
                return;
            }
            
            indiceMemoria=direccionLogicaAFisica(vm->registros[0]+1, vm->segmentos); //Obtenemos el indice donde comienza el opB
            if (indiceMemoria==-1 || indiceMemoria>=TAM_MEMORIA){    //Condiciones de corte
                vm->registros[0] = -1;
                vm->corriendo = 0;
                continue; //vuelve a chequear el while para salir
            }
            opB=0;
            for (i=0; i<tipoOpB; i++){  //Como el tipo determina la cantidad de bytes del operando, leemos tipoOpB celdas de memoria
                opB = opB << 8;
                if (indiceMemoria >= TAM_MEMORIA){
                    printf("Fallo de segmento\n");
                    vm->corriendo = 0;
                    break;
                }
                opB += vm->memoria.datos[indiceMemoria]; 
                indiceMemoria++;
            }
            
            if (!vm->corriendo)
                continue;

            opA=0;
            for (i=0; i<tipoOpA; i++){
                opA = opA << 8;
                if (indiceMemoria >= TAM_MEMORIA){
                    printf("Fallo de segmento\n");
                    vm->corriendo = 0;
                    break;
                }
                opA += vm->memoria.datos[indiceMemoria]; 
                indiceMemoria++;
            }

            if (!vm->corriendo)
                continue;

            vm->registros[0] += tamanoInstruccion;
            vm->registros[2] = (tipoOpA << 24) | (opA & 0x00FFFFFF);   // OP1
            vm->registros[3] = (tipoOpB << 24) | (opB & 0x00FFFFFF);   // OP2
            tabla_instrucciones[vm->registros[1]](vm);
        
        }
    }
}

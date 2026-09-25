#include <string.h>
#include "instrucciones.h"
#include "tipos.h"
#include <stdio.h>
#include "Registros.h"
#include <stdint.h>
#include "funciones.h"
static const char *const mnemonicos[32] = {
    [0x00] = "SYS",  [0x01] = "JMP",  [0x02] = "JP",   [0x03] = "JN",
    [0x04] = "JZ",   [0x05] = "JC",   [0x06] = "JV",   [0x07] = "JNP",
    [0x08] = "JNN",  [0x09] = "JNZ",  [0x0A] = "NOT",
    [0x0F] = "STOP",
    [0x10] = "MOV",  [0x11] = "ADD",  [0x12] = "SUB",  [0x13] = "MUL",
    [0x14] = "DIV",  [0x15] = "CMP",  [0x16] = "AND",  [0x17] = "OR",
    [0x18] = "XOR",  [0x19] = "SWAP", [0x1A] = "SHL",  [0x1B] = "SHR",
    [0x1C] = "SAR",  [0x1D] = "LDL",  [0x1E] = "LDH",  [0x1F] = "RND",
};
static const char *const nombresRegistros[32] = {
    [REG_IP]  = "IP",  [REG_OPC] = "OPC", [REG_OP1] = "OP1", [REG_OP2] = "OP2",
    [REG_LAR] = "LAR", [REG_MAR] = "MAR", [REG_MBR] = "MBR",
    [REG_EAX] = "EAX", [REG_EBX] = "EBX", [REG_ECX] = "ECX", [REG_EDX] = "EDX",
    [REG_EEX] = "EEX", [REG_EFX] = "EFX", [REG_AC]  = "AC",  [REG_CC]  = "CC",
    [REG_CS]  = "CS",  [REG_DS]  = "DS",
};
const char*buscaMNEM(int codigo) {
    if (codigo < 0 || codigo >= 32 || mnemonicos[codigo] == NULL)
        return "instruccion no identificada";
    return mnemonicos[codigo];
}

const char *buscaRegistro(uint32_t op) {
    uint32_t codigo = op & 0x1F;
    return nombresRegistros[codigo] ? nombresRegistros[codigo] : "registro no encontrado";
}

void imprimirOperando(uint32_t tipo,uint32_t operando){

    int32_t desplazamiento, valor;
    uint32_t  codigoRegistro;
    const char *nombreDelRegistro;
    
    
    if (tipo == 1 ) {        //Registro
        codigoRegistro= operando & 0x1F;  
        nombreDelRegistro = buscaRegistro(codigoRegistro);
        printf("%s",nombreDelRegistro);
    }  
    else
        if (tipo == 2){       //Inmediato
            valor = (int32_t)((operando & 0xFFFFu) << 16);
            valor = valor >> 16;
            printf("%d",valor);
        }
        else
            if (tipo ==3){                  //Memoria
            codigoRegistro= operando & 0x1F;  // 5 bits de mas abajo
            desplazamiento =(operando >> 8) & 0xFFFF; // los 16 bits de arriba
            nombreDelRegistro = buscaRegistro(codigoRegistro);
            //por si arrastra el signo (igual que en funcion direccion logica a fisica)
            desplazamiento = desplazamiento << 16;
            desplazamiento = desplazamiento >> 16;
        
            printf("[%s%+d]", nombreDelRegistro, desplazamiento);
            }
            else
                printf("Operando inexistente");
        
}
void disassembler(MaquinaVirtual *vm){

    uint8_t instruccion;
    int32_t cantOp,tipoOpA, tipoOpB, indiceMemoria, i, direccionFisica;
    uint32_t opB, opA,pos = 0;
    uint32_t tamanoCodigo = (uint32_t)vm->segmentos[0] & 0xFFFF;
    
    while(pos<tamanoCodigo){  // procesa mientras corriendo sea 1, cuando encuentra STOP cambia corriendo a 0 y termina la ejecucion
        direccionFisica=direccionLogicaAFisica(pos, vm->segmentos);
        if (direccionFisica==-1 || direccionFisica>=TAM_MEMORIA){    //Condiciones de corte
            pos=tamanoCodigo;
            continue; //vuelve a chequear el while para salir
        }
        instruccion= vm->memoria.datos[direccionFisica];

        vm->registros[REG_OPC]= instruccion & 0x1F;
        cantOp= cantidadOperandosALeer(vm->registros[REG_OPC]);
        if (!cantOp){             //Cuando encuentra STOP pone corriendo en 0 y termina la ejecucion
            printf("[%04X] ", direccionFisica);
            printf("%02X ", vm->memoria.datos[direccionFisica]);
            printf("| %s\n", buscaMNEM(vm->registros[REG_OPC]));
            pos += 1;
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
            
            uint32_t tamanoInstruccion = 1 + (uint32_t)tipoOpA + (uint32_t)tipoOpB;

            if (tamanoInstruccion > tamanoCodigo - pos) {
                printf("Error: Se sale del segmento de codigo\n");
                return;
            }

            indiceMemoria=direccionLogicaAFisica(pos+1, vm->segmentos); //Obtenemos el indice donde comienza el opB
            if (indiceMemoria < 0 ||indiceMemoria >= TAM_MEMORIA) {
                printf("Error: el indice se sale de la memoria\n");
                return;
            }
            printf("[%04X] ", direccionFisica);
            

            opB=0;
            for (i=0; i<tipoOpB; i++){  //Como el tipo determina la cantidad de bytes del operando, leemos tipoOpB celdas de memoria
                opB = opB << 8;
                if (indiceMemoria >= TAM_MEMORIA){
                    printf("Fallo de segmento\n");
                    return;
                }
                opB += vm->memoria.datos[indiceMemoria]; 
                indiceMemoria++;
            }
            
            opA=0;
            for (i=0; i<tipoOpA; i++){
                opA = opA << 8;
                if (indiceMemoria >= TAM_MEMORIA){
                    printf("Fallo de segmento\n");
                    return;
                }
                opA += vm->memoria.datos[indiceMemoria]; 
                indiceMemoria++;
            }
            
            for(i=0;i<(int32_t)tamanoInstruccion;i++){
                printf("%02X ", vm->memoria.datos[direccionFisica+i]);
            }
            pos += tamanoInstruccion;
            
            printf("| %s ",buscaMNEM(vm->registros[REG_OPC]));

            imprimirOperando(tipoOpA,opA);
            if (cantOp==2){
                printf(", ");
                imprimirOperando(tipoOpB,opB);
            
            }
            printf("\n");
        }
    
    }
}

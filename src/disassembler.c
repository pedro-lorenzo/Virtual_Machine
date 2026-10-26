#include <string.h>
#include "instrucciones.h"
#include "tipos.h"
#include <stdio.h>
#include "Registros.h"
#include <stdint.h>
#include "funciones.h"

#define MAX_BYTES_INSTRUCCION 7   // 1 byte de opcode + hasta 3 bytes por operando
static const char *const mnemonicos[32] = {
    [0x00] = "SYS",  [0x01] = "JMP",  [0x02] = "JP",   [0x03] = "JN",
    [0x04] = "JZ",   [0x05] = "JC",   [0x06] = "JV",   [0x07] = "JNP",
    [0x08] = "JNN",  [0x09] = "JNZ",  [0x0A] = "NOT",  [0x0B] = "PUSH", 
    [0x0C] = "POP",  [0x0D] = "CALL", [0x0E] = "RET",  [0x0F] = "STOP",
    [0x10] = "MOV",  [0x11] = "ADD",  [0x12] = "SUB",  [0x13] = "MUL",
    [0x14] = "DIV",  [0x15] = "CMP",  [0x16] = "AND",  [0x17] = "OR",
    [0x18] = "XOR",  [0x19] = "SWAP", [0x1A] = "SHL",  [0x1B] = "SHR",
    [0x1C] = "SAR",  [0x1D] = "LDL",  [0x1E] = "LDH",  [0x1F] = "RND",
};
static const char *const nombresRegistros[32] = {
    [REG_IP]  = "IP",  [REG_OPC] = "OPC", [REG_OP1] = "OP1", [REG_OP2] = "OP2",
    [REG_LAR] = "LAR", [REG_MAR] = "MAR", [REG_MBR] = "MBR",
    [REG_SP]  = "SP",  [REG_BP]  = "BP",
    [REG_EAX] = "EAX", [REG_EBX] = "EBX", [REG_ECX] = "ECX", [REG_EDX] = "EDX",
    [REG_EEX] = "EEX", [REG_EFX] = "EFX", [REG_AC]  = "AC",  [REG_CC]  = "CC",
    [REG_CS]  = "CS",  [REG_DS]  = "DS",
    [REG_ES]  = "ES",  [REG_SS]  = "SS",  [REG_KS]  = "KS",  [REG_PS]  = "PS",
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

/* Nombre con el que se muestra un operando de registro. Para EAX..EFX el sector
   elige el "pseudonimo": 00 = EAX, 01 = AL, 10 = AH, 11 = AX (y asi con la letra de cada registro).
   El resto de los registros no tiene sectores y se muestra con su nombre completo. */
static void nombreRegistroOperando(uint32_t codigo, uint32_t sector, char *nombre){
    if (codigo >= REG_EAX && codigo <= REG_EFX && sector != 0){
        nombre[0] = (char)('A' + (codigo - REG_EAX));
        nombre[1] = (sector == 1) ? 'L' : (sector == 2) ? 'H' : 'X';
        nombre[2] = '\0';
    }
    else
        strcpy(nombre, buscaRegistro(codigo));
}

void imprimirOperando(uint32_t tipo,uint32_t operando){

    int32_t desplazamiento, valor;
    uint32_t codigoRegistro, modificador;
    const char *prefijo;
    char nombre[8];

    if (tipo == 1 ) {        //Registro
        codigoRegistro = operando & 0x1F;
        nombreRegistroOperando(codigoRegistro, sectorRegistro(operando), nombre);
        printf("%s", nombre);
    }
    else
        if (tipo == 2){       //Inmediato
            valor = (int32_t)((operando & 0xFFFFu) << 16);
            valor = valor >> 16;
            printf("%d",valor);
        }
        else
            if (tipo ==3){                  //Memoria
                codigoRegistro = operando & 0x1F;  // 5 bits de mas abajo
                modificador = (operando >> 5) & 0x7;
                prefijo = (modificador == 1) ? "b" : (modificador == 2) ? "w" : (modificador == 4) ? "l" : "";
                desplazamiento = (int32_t)(((operando >> 8) & 0xFFFFu) << 16);   // 16 bits de arriba, con signo
                desplazamiento = desplazamiento >> 16;
                printf("%s[%s%+d]", prefijo, buscaRegistro(codigoRegistro), desplazamiento);
            }
            else
                printf("Operando inexistente");
}

/* Muestra las cadenas del Const Segment, una por linea. Va antes del codigo
   porque en la memoria el Const Segment esta antes que el Code Segment. */
static void imprimirConstantes(MaquinaVirtual *vm){
    uint32_t ks, base, tamano, inicio, fin, largo, mostrados, i;
    uint8_t caracter;

    ks = (uint32_t)vm->registros[REG_KS];
    if (ks == 0xFFFFFFFFu || (ks >> 16) >= CANT_SEGMENTOS)
        return;
    base = (uint32_t)vm->segmentos[ks >> 16] >> 16;
    tamano = (uint32_t)vm->segmentos[ks >> 16] & 0xFFFF;

    inicio = 0;
    while (inicio < tamano){
        fin = inicio;
        while (fin < tamano && vm->memoria.datos[base + fin] != 0)
            fin++;
        largo = fin - inicio + (fin < tamano ? 1 : 0);     // incluye el '\0' si lo tiene

        printf("[%04X] ", base + inicio);
        mostrados = largo;
        if (largo > MAX_BYTES_INSTRUCCION)                   // si no entra, los primeros 6 y ".."
            mostrados = MAX_BYTES_INSTRUCCION - 1;
        for (i = 0; i < mostrados; i++)
            printf("%02X ", vm->memoria.datos[base + inicio + i]);
        if (largo > MAX_BYTES_INSTRUCCION){
            printf(".. ");
            mostrados++;
        }
        printf("%*s", 3 * (int32_t)(MAX_BYTES_INSTRUCCION - mostrados), "");

        printf("| \"");
        for (i = inicio; i < fin; i++){
            caracter = vm->memoria.datos[base + i];
            printf("%c", (caracter >= 32 && caracter <= 126) ? caracter : '.');
        }
        printf("\"\n");
        inicio += largo;
    }
}

void disassembler(MaquinaVirtual *vm){

    uint8_t instruccion;
    int32_t cantOp,tipoOpA, tipoOpB, indiceMemoria, i, direccionFisica;
    uint32_t opB, opA,pos = 0;     // pos: offset dentro del Code Segment
    uint32_t cs = (uint32_t)vm->registros[REG_CS];
    uint32_t tamanoCodigo = (uint32_t)vm->segmentos[cs >> 16] & 0xFFFF;
    
    imprimirConstantes(vm);
    while(pos<tamanoCodigo){  // procesa mientras corriendo sea 1, cuando encuentra STOP cambia corriendo a 0 y termina la ejecucion
        direccionFisica=direccionLogicaAFisica(cs + pos, vm->segmentos, vm->memoria.tamano);
        if (direccionFisica==-1 || (uint32_t)direccionFisica>=vm->memoria.tamano){    //Condiciones de corte
            pos=tamanoCodigo;
            continue; //vuelve a chequear el while para salir
        }
        instruccion= vm->memoria.datos[direccionFisica];

        vm->registros[REG_OPC]= instruccion & 0x1F;
        cantOp= cantidadOperandosALeer(vm->registros[REG_OPC]);
        if (!cantOp){             //Cuando encuentra STOP pone corriendo en 0 y termina la ejecucion
            if (cs + pos == (uint32_t)vm->registros[REG_IP])    // entry point
                printf(">");
            printf("[%04X] ", direccionFisica);
            printf("%02X ", vm->memoria.datos[direccionFisica]);
            printf("%*s", 3 * (MAX_BYTES_INSTRUCCION - 1), "");
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

            indiceMemoria=direccionLogicaAFisica(cs + pos + 1, vm->segmentos, vm->memoria.tamano); //Obtenemos el indice donde comienza el opB
            if (indiceMemoria < 0 || (uint32_t)indiceMemoria >= vm->memoria.tamano) {
                printf("Error: el indice se sale de la memoria\n");
                return;
            }
            if (cs + pos == (uint32_t)vm->registros[REG_IP])    // entry point
                printf(">");
                
            printf("[%04X] ", direccionFisica);
            

            opB=0;
            for (i=0; i<tipoOpB; i++){  //Como el tipo determina la cantidad de bytes del operando, leemos tipoOpB celdas de memoria
                opB = opB << 8;
                if ((uint32_t)indiceMemoria >= vm->memoria.tamano){
                    printf("Fallo de segmento\n");
                    return;
                }
                opB += vm->memoria.datos[indiceMemoria]; 
                indiceMemoria++;
            }
            
            opA=0;
            for (i=0; i<tipoOpA; i++){
                opA = opA << 8;
                if ((uint32_t)indiceMemoria >= vm->memoria.tamano){
                    printf("Fallo de segmento\n");
                    return;
                }
                opA += vm->memoria.datos[indiceMemoria]; 
                indiceMemoria++;
            }
            
            for(i=0;i<(int32_t)tamanoInstruccion;i++){
                printf("%02X ", vm->memoria.datos[direccionFisica+i]);
            }
            printf("%*s", 3 * (MAX_BYTES_INSTRUCCION - (int32_t)tamanoInstruccion), "");
            pos += tamanoInstruccion;
            
            printf("| %-5s",buscaMNEM(vm->registros[REG_OPC]));

            imprimirOperando(tipoOpA,opA);
            if (cantOp==2){
                printf(", ");
                imprimirOperando(tipoOpB,opB);
            
            }
            printf("\n");
        }
    
    }
}
#include <stdio.h>
#include "instrucciones.h"

/* ============================================================
   TABLA DE DESPACHO
   Se indexa con el opcode de 5 bits (registros[OPC], ya
   enmascarado con 0x1F). Los 32 casilleros quedan cubiertos:

     0x00-0x0A  -> instrucciones de un operando (11)
     0x0B-0x0E  -> huecos sin usar -> instruccion invalida (4)
     0x0F       -> STOP (sin operandos)
     0x10-0x1F  -> instrucciones de dos operandos (16)
   ============================================================ */
InstruccionFn tabla_instrucciones[32] = {

    /* ---- un operando ---- */
    [0x00] = ejecutarSYS,
    [0x01] = ejecutarJMP,
    [0x02] = ejecutarJP,
    [0x03] = ejecutarJN,
    [0x04] = ejecutarJZ,
    [0x05] = ejecutarJC,
    [0x06] = ejecutarJV,
    [0x07] = ejecutarJNP,
    [0x08] = ejecutarJNN,
    [0x09] = ejecutarJNZ,
    [0x0A] = ejecutarNOT,

    /* ---- huecos: no existen en la especificacion ---- */
    [0x0B] = instruccionInvalida,
    [0x0C] = instruccionInvalida,
    [0x0D] = instruccionInvalida,
    [0x0E] = instruccionInvalida,

    /* ---- sin operandos ---- */
    [0x0F] = ejecutarSTOP,

    /* ---- dos operandos ---- */
    [0x10] = ejecutarMOV,
    [0x11] = ejecutarADD,
    [0x12] = ejecutarSUB,
    [0x13] = ejecutarMUL,
    [0x14] = ejecutarDIV,
    [0x15] = ejecutarCMP,
    [0x16] = ejecutarAND,
    [0x17] = ejecutarOR,
    [0x18] = ejecutarXOR,
    [0x19] = ejecutarSWAP,
    [0x1A] = ejecutarSHL,
    [0x1B] = ejecutarSHR,
    [0x1C] = ejecutarSAR,
    [0x1D] = ejecutarLDL,
    [0x1E] = ejecutarLDH,
    [0x1F] = ejecutarRND,
};

/* ============================================================
   IMPLEMENTACIONES

   Por ahora son placeholders: solo sirven para confirmar que el
   despacho llega a la funcion correcta (probalo llamando
   tabla_instrucciones[registros[1]](vm) desde procesador.c y
   viendo que imprime el mnemonico esperado). STOP e
   instruccionInvalida ya estan resueltas porque la espec las fija
   sin ambiguedad; el resto queda con TODO para que las diseñemos
   una por una.
   ============================================================ */

void ejecutarSTOP(MaquinaVirtual *vm){
    printf("STOP\n");
    vm->registros.registros[0] = -1;   /* IP = 0xFFFFFFFF */
    vm->corriendo = 0;
}

void instruccionInvalida(MaquinaVirtual *vm){
    printf("Error: instruccion invalida\n");
    vm->corriendo = 0;
}

void ejecutarSYS (MaquinaVirtual *vm){ printf("SYS\n");  /* TODO */ }
void ejecutarJMP (MaquinaVirtual *vm){ printf("JMP\n");  /* TODO */ }
void ejecutarJP  (MaquinaVirtual *vm){ printf("JP\n");   /* TODO */ }
void ejecutarJN  (MaquinaVirtual *vm){ printf("JN\n");   /* TODO */ }
void ejecutarJZ  (MaquinaVirtual *vm){ printf("JZ\n");   /* TODO */ }
void ejecutarJC  (MaquinaVirtual *vm){ printf("JC\n");   /* TODO */ }
void ejecutarJV  (MaquinaVirtual *vm){ printf("JV\n");   /* TODO */ }
void ejecutarJNP (MaquinaVirtual *vm){ printf("JNP\n");  /* TODO */ }
void ejecutarJNN (MaquinaVirtual *vm){ printf("JNN\n");  /* TODO */ }
void ejecutarJNZ (MaquinaVirtual *vm){ printf("JNZ\n");  /* TODO */ }
void ejecutarNOT (MaquinaVirtual *vm){ printf("NOT\n");  /* TODO */ }

void ejecutarMOV (MaquinaVirtual *vm){ printf("MOV\n");  /* TODO */ }
void ejecutarADD (MaquinaVirtual *vm){ printf("ADD\n");  /* TODO */ }
void ejecutarSUB (MaquinaVirtual *vm){ printf("SUB\n");  /* TODO */ }
void ejecutarMUL (MaquinaVirtual *vm){ printf("MUL\n");  /* TODO */ }
void ejecutarDIV (MaquinaVirtual *vm){ printf("DIV\n");  /* TODO */ }
void ejecutarCMP (MaquinaVirtual *vm){ printf("CMP\n");  /* TODO */ }
void ejecutarAND (MaquinaVirtual *vm){ printf("AND\n");  /* TODO */ }
void ejecutarOR  (MaquinaVirtual *vm){ printf("OR\n");   /* TODO */ }
void ejecutarXOR (MaquinaVirtual *vm){ printf("XOR\n");  /* TODO */ }
void ejecutarSWAP(MaquinaVirtual *vm){ printf("SWAP\n"); /* TODO: 3 XOR, ver espec */ }
void ejecutarSHL (MaquinaVirtual *vm){ printf("SHL\n");  /* TODO */ }
void ejecutarSHR (MaquinaVirtual *vm){ printf("SHR\n");  /* TODO */ }
void ejecutarSAR (MaquinaVirtual *vm){ printf("SAR\n");  /* TODO */ }
void ejecutarLDL (MaquinaVirtual *vm){ printf("LDL\n");  /* TODO */ }
void ejecutarLDH (MaquinaVirtual *vm){ printf("LDH\n");  /* TODO */ }
void ejecutarRND (MaquinaVirtual *vm){ printf("RND\n");  /* TODO */ }

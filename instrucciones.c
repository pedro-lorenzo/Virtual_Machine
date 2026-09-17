#include <stdio.h>
#include "instrucciones.h"
#include "instrucciones.h"
#include "funciones.h"
#include "Registros.h"
#include "cc.h"

/* TABLA DE OPERACIONES
   Se indexa con el opcode de 5 bits (registros[OPC], ya
   enmascarado con 0x1F). Consta de un vector de punteros a funciones
   cuyo indice justamente sera el codigo de operacion.
   Las 32 posiciones del vector quedan cubiertas:

     0x00-0x0A  -> instrucciones de un operando (11)
     0x0B-0x0E  -> huecos sin usar -> instruccion invalida (4)
     0x0F       -> STOP (sin operandos)
     0x10-0x1F  -> instrucciones de dos operandos (16)
    */
InstruccionFn tabla_instrucciones[32] = {
    /* ---- sin operandos ---- */
    [0x0F] = ejecutarSTOP,
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


void instruccionInvalida(MaquinaVirtual *vm){
    printf("Error: instruccion invalida\n");
    vm->corriendo = 0;
}

void ejecutarSTOP(MaquinaVirtual *vm){
    printf("STOP\n");
    vm->registros[0] = -1;   // IP = 0xFFFFFFFF
    vm->corriendo = 0;
}

//  ARITMETICAS


void ejecutarADD(MaquinaVirtual *vm) {
    int valorA, valorB, escribioOk;
    int64_t sumaConSigno;
    uint64_t sumaSinSigno;
    int32_t resultado;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    valorB = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    /* Suma en 64 bits para poder ver, antes de truncar, si el
       resultado se pasa de los 32 bits disponibles (evita el
       overflow con signo, que en C es comportamiento indefinido). */
    sumaConSigno = (int64_t)valorA + (int64_t)valorB;
    sumaSinSigno = (uint64_t)(uint32_t)valorA + (uint64_t)(uint32_t)valorB;
    resultado = (int32_t)sumaConSigno;

    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }

    actualizarCC_sumaResta(vm, sumaConSigno, sumaSinSigno, resultado);
}

void ejecutarSUB(MaquinaVirtual *vm) {
    int valorA, valorB, escribioOk;
    int64_t sumaConSigno;
    uint32_t menosBSinSigno;
    uint64_t sumaSinSigno;
    int32_t resultado;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    valorB = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    /* Se calcula como A + (-B) via complemento a 2 
        por eso comparte el mismo criterio de C y V
       que ADD. La resta con signo se hace en 64 bits directamente
       (sin negar B por separado) para no pisar el caso B == INT_MIN,
       donde -B por si solo ya desborda un int de 32 bits. */
    sumaConSigno = (int64_t)valorA - (int64_t)valorB;

    /* Para el carry sin signo si hace falta el -B "de verdad", pero
       calculado en aritmetica unsigned (mod 2^32), que nunca es UB. */
    menosBSinSigno = (uint32_t)(0u - (uint32_t)valorB);
    sumaSinSigno = (uint64_t)(uint32_t)valorA + (uint64_t)menosBSinSigno;

    resultado = (int32_t)sumaConSigno;

    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }

    actualizarCC_sumaResta(vm, sumaConSigno, sumaSinSigno, resultado);
}

void ejecutarMUL(MaquinaVirtual *vm) {
    int valorA, valorB, escribioOk;
    int64_t productoConSigno;
    uint64_t productoSinSigno;
    int32_t resultado;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    valorB = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    /* 64 bits alcanzan siempre: el peor caso (INT_MIN * INT_MIN)
       entra comodo en un int64_t. */
    productoConSigno = (int64_t)valorA * (int64_t)valorB;
    productoSinSigno = (uint64_t)(uint32_t)valorA * (uint64_t)(uint32_t)valorB;
    resultado = (int32_t)productoConSigno;

    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }

    actualizarCC_producto(vm, productoConSigno, productoSinSigno, resultado);
}

void ejecutarDIV(MaquinaVirtual *vm) {
    int valorA, valorB, overflow, escribioOk;
    int32_t cociente, resto;
    int64_t cocienteAmplio;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    valorB = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    if (valorB == 0) {
        /* Division por cero: error fatal segun la especificacion. */
        printf("Error: division por cero\n");
        vm->corriendo = 0;
        return;
    }

    /* Unico caso de overflow posible en division de enteros de 32
       bits: INT_MIN / -1 (el resultado matematico, 2147483648, no
       entra en el rango con signo). No es un error fatal segun la
       especificacion: se trunca a 32 bits y se prende V. */
    overflow = (valorA == INT32_MIN && valorB == -1);

    if (overflow) {
        cocienteAmplio = (int64_t)valorA / (int64_t)valorB;
        cociente = (int32_t)cocienteAmplio; /* truncado a 32 bits */
        resto = 0;                          /* INT_MIN es multiplo exacto de -1 */
    } else {
        cociente = valorA / valorB;
        resto = valorA % valorB;
    }

    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], cociente);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }
    vm->registros[REG_AC] = resto;

    /* C siempre 0 en DIV: no hay sumador involucrado. */
    actualizarCC_division(vm, cociente, overflow);
}

//  LOGICAS

void ejecutarAND(MaquinaVirtual *vm) {
    int valorA, valorB, escribioOk;
    int32_t resultado;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    valorB = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    resultado = valorA & valorB;

    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }

    actualizarCC_logica(vm, resultado);
}

void ejecutarOR(MaquinaVirtual *vm) {
    int valorA, valorB, escribioOk;
    int32_t resultado;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    valorB = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    resultado = valorA | valorB;

    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }

    actualizarCC_logica(vm, resultado);
}

void ejecutarXOR(MaquinaVirtual *vm) {
    int valorA, valorB, escribioOk;
    int32_t resultado;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    valorB = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    resultado = valorA ^ valorB;

    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }

    actualizarCC_logica(vm, resultado);
}

void ejecutarNOT(MaquinaVirtual *vm) {
    int valor, escribioOk;
    int32_t resultado;

    valor = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;

    resultado = ~valor;

    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }

    actualizarCC_logica(vm, resultado);
}

//  SWAP -- se implementa reutilizando ejecutarXOR tres veces, intercambiando OP1/OP2 entre llamadas.

void ejecutarSWAP(MaquinaVirtual *vm) {
    unsigned int op1Original, op2Original;

    op1Original = vm->registros[REG_OP1];
    op2Original = vm->registros[REG_OP2];

    ejecutarXOR(vm);
    if (!vm->corriendo) return;

    vm->registros[REG_OP1] = op2Original;
    vm->registros[REG_OP2] = op1Original;
    ejecutarXOR(vm);
    if (!vm->corriendo) return;

    vm->registros[REG_OP1] = op1Original;
    vm->registros[REG_OP2] = op2Original;
    ejecutarXOR(vm);
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

void ejecutarMOV (MaquinaVirtual *vm){ printf("MOV\n");  /* TODO */ }
void ejecutarCMP (MaquinaVirtual *vm){ printf("CMP\n");  /* TODO */ }
void ejecutarSHL (MaquinaVirtual *vm){ printf("SHL\n");  /* TODO */ }
void ejecutarSHR (MaquinaVirtual *vm){ printf("SHR\n");  /* TODO */ }
void ejecutarSAR (MaquinaVirtual *vm){ printf("SAR\n");  /* TODO */ }
void ejecutarLDL (MaquinaVirtual *vm){ printf("LDL\n");  /* TODO */ }
void ejecutarLDH (MaquinaVirtual *vm){ printf("LDH\n");  /* TODO */ }
void ejecutarRND (MaquinaVirtual *vm){ printf("RND\n");  /* TODO */ }
#include <stdio.h>
#include <stdlib.h>
#include "instrucciones.h"
#include "funciones.h"
#include "Registros.h"
#include "cc.h"
#include <errno.h>
#include <string.h>
#include "imagen.h"

/* TABLA DE OPERACIONES
   Se indexa con el opcode de 5 bits (registros[OPC], ya
   enmascarado con 0x1F). Consta de un vector de punteros a funciones
   cuyo indice justamente sera el codigo de operacion.
   Las 32 posiciones del vector quedan cubiertas:

     0x00-0x0A  -> instrucciones de un operando (11)
     0x0B-0x0D  -> PUSH, POP, CALL (un operando)
     0x0E       -> RET (sin operandos)
     0x0F       -> STOP (sin operandos)
     0x10-0x1F  -> instrucciones de dos operandos (16)
*/
InstruccionFn tabla_instrucciones[32] = {
    // sin operandos
    [0x0F] = ejecutarSTOP,

    // Un operando
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

    // Pila
    [0x0B] = ejecutarPUSH,
    [0x0C] = ejecutarPOP,
    [0x0D] = ejecutarCALL,
    [0x0E] = ejecutarRET,      // sin operandos
    
    // Dos operandos
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
    printf("\n");
    vm->registros[REG_IP] = -1;   // IP = 0xFFFFFFFF
    vm->corriendo = 0;
}

//  PILA
/* SP y SS son direcciones logicas con el mismo codigo de segmento en la parte alta,
   asi que se pueden comparar directamente. Siempre se mueven 4 bytes, en big endian
   (escribirMemoria/leerMemoria ya lo hacen asi). */

static int apilar(MaquinaVirtual *vm, int32_t valor){
    uint32_t sp = (uint32_t)vm->registros[REG_SP] - 4;
    vm->registros[REG_SP] = (int32_t)sp;

    if (vm->registros[REG_SS] == -1 || sp < (uint32_t)vm->registros[REG_SS]){
        printf("Error: stack overflow\n");
        vm->corriendo = 0;
        return 0;
    }
    if (!escribirMemoria(vm, sp, 4, valor)){
        vm->corriendo = 0;
        return 0;
    }
    return 1;
}

static int desapilar(MaquinaVirtual *vm, int32_t *valor){
    uint32_t sp = (uint32_t)vm->registros[REG_SP];
    uint32_t ss = (uint32_t)vm->registros[REG_SS];
    uint32_t tamanoPila;

    if (ss == 0xFFFFFFFFu || (ss >> 16) >= CANT_SEGMENTOS){
        printf("Error: stack underflow\n");
        vm->corriendo = 0;
        return 0;
    }
    tamanoPila = (uint32_t)vm->segmentos[ss >> 16] & 0xFFFFu;

    // underflow: SP fuera del Stack Segment o menos de 4 bytes hasta el final de la pila
    if ((sp >> 16) != (ss >> 16) || (sp & 0xFFFFu) + 4 > tamanoPila){
        printf("Error: stack underflow\n");
        vm->corriendo = 0;
        return 0;
    }
    if (!leerMemoria(vm, sp, 4, valor)){
        vm->corriendo = 0;
        return 0;
    }
    return 1;
}

void ejecutarPUSH(MaquinaVirtual *vm){
    int32_t valor;
    uint32_t sp = (uint32_t)vm->registros[REG_SP] - 4;

    // 1-2: decrementar SP y controlar overflow antes de leer el operando
    vm->registros[REG_SP] = (int32_t)sp;
    if (vm->registros[REG_SS] == -1 || sp < (uint32_t)vm->registros[REG_SS]){
        printf("Error: stack overflow\n");
        vm->corriendo = 0;
        return;
    }
    // 3-4: leerOperando ya devuelve el valor extendido a 4 bytes con signo
    valor = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
    // 5: guardar en el tope (byte mas significativo en SP)
    if (!escribirMemoria(vm, sp, 4, valor))
        vm->corriendo = 0;
}

void ejecutarPOP(MaquinaVirtual *vm){
    int32_t valor;

    if (!desapilar(vm, &valor)) return;
    // escribirOperando trunca segun el sector o el modificador del operando
    if (!escribirOperando(vm, vm->registros[REG_OP1], valor) || !vm->corriendo){
        vm->corriendo = 0;
        return;
    }
    vm->registros[REG_SP] += 4;
}

void ejecutarCALL(MaquinaVirtual *vm){
    int32_t destino;
    uint32_t ip;

    // PUSH IP (ya apunta a la instruccion siguiente) ...
    if (!apilar(vm, vm->registros[REG_IP])) return;
    // ... y JMP: se reemplazan los 2 bytes bajos del IP por el operando
    destino = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
    ip = (uint32_t)vm->registros[REG_IP];
    vm->registros[REG_IP] = (int32_t)((ip & 0xFFFF0000u) | ((uint32_t)destino & 0xFFFFu));
}

void ejecutarRET(MaquinaVirtual *vm){
    int32_t valor;

    // POP IP. Si la subrutina principal retorna, IP queda en -1 y termina el proceso.
    if (!desapilar(vm, &valor)) return;
    vm->registros[REG_IP] = valor;
    vm->registros[REG_SP] += 4;
}

//  ARITMETICAS

void ejecutarADD(MaquinaVirtual *vm) {
    int32_t valorA, valorB, escribioOk;
    int64_t sumaConSigno;
    uint64_t sumaSinSigno;
    int32_t resultado;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
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
    int32_t valorA, valorB, escribioOk;
    int64_t sumaConSigno;
    uint32_t menosBSinSigno;
    uint64_t sumaSinSigno;
    int32_t resultado;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
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
    int32_t valorA, valorB, escribioOk;
    int64_t productoConSigno;
    uint64_t productoSinSigno;
    int32_t resultado;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
    valorB = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    /* 64 bits alcanzan siempre: el peor caso (INT_MIN * INT_MIN)
       entra en un int64_t. */
    productoConSigno = (int64_t)valorA * (int64_t)valorB;
    productoSinSigno = (uint64_t)(uint32_t)valorA * (uint64_t)(uint32_t)valorB;
    resultado = (int32_t)productoConSigno;

    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }

    actualizarCC_producto(vm, productoConSigno, productoSinSigno, resultado);
}

void ejecutarDIV(MaquinaVirtual *vm) {
    int32_t valorA, valorB, overflow, escribioOk;
    int32_t cociente, resto;
    int64_t cocienteAmplio;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
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
    int32_t valorA, valorB, escribioOk;
    int32_t resultado;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
    valorB = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    resultado = valorA & valorB;

    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }

    actualizarCC_logica(vm, resultado);
}

void ejecutarOR(MaquinaVirtual *vm) {
    int32_t valorA, valorB, escribioOk;
    int32_t resultado;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
    valorB = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    resultado = valorA | valorB;

    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }

    actualizarCC_logica(vm, resultado);
}

void ejecutarXOR(MaquinaVirtual *vm) {
    int32_t valorA, valorB, escribioOk;
    int32_t resultado;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
    valorB = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    resultado = valorA ^ valorB;

    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }

    actualizarCC_logica(vm, resultado);
}

void ejecutarNOT(MaquinaVirtual *vm) {
    int32_t valor, escribioOk;
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
    uint32_t op1Original, op2Original, valorActual;

    op1Original = vm->registros[REG_OP1];
    op2Original = vm->registros[REG_OP2];

    if (op1Original == op2Original) {
        valorActual = leerOperando(vm, op1Original);
        if (!vm->corriendo) return;
        actualizarCC_logica(vm, valorActual);
        return;
    }

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

/* 
   SHL -> equivale a valorA * 2^n. Se calcula en 64 bits para
   comparar contra el rango de 32 antes de truncar (mismo patron
   que MUL). Caso especial n>=32
*/
void ejecutarSHL(MaquinaVirtual *vm) {
    int32_t valorA, n, resultado;
    int escribioOk, C, V;
    int64_t productoConSigno;
    uint64_t productoSinSigno;
 
    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
    n = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;
 
    if (n < 0) {
        printf("Error: desplazamiento negativo\n");
        vm->corriendo = 0;
        return;
    }

    if (n >= 32) {
        resultado = 0;
        C = (valorA != 0) ? 1 : 0;
        V = C;
    } else {
        productoConSigno = (int64_t)valorA * (int64_t)(1LL << n); // castea el 1 a 64 bits para evitar error de ejecucion
        productoSinSigno = (uint64_t)(uint32_t)valorA << n;
        resultado = (int32_t)productoConSigno;
        C = (productoSinSigno > 0xFFFFFFFFULL) ? 1 : 0;
        V = (productoConSigno > INT32_MAX || productoConSigno < INT32_MIN) ? 1 : 0;
    }
 
    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }
 
    actualizarCC_desplazamiento(vm, resultado, C, V);
}
 
/* 
   SHR ->  Nunca hay overflow ni carry: un corrimiento a 
   la derecha solo puede achicar la magnitud, jamas hacer 
   que el resultado "exceda 32 bits".
   
*/
void ejecutarSHR(MaquinaVirtual *vm) {
    int32_t valorA, n, resultado;
    int escribioOk;
 
    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
    n = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    if (n < 0) {
        printf("Error: desplazamiento negativo\n");
        vm->corriendo = 0;
        return;
    }
 
    if (n >= 32)
        resultado = 0;
    else
        resultado = (int32_t)((uint32_t)valorA >> n);
 
    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }
 
    actualizarCC_desplazamiento(vm, resultado, 0, 0);
}
 
/* 
   SAR -> igual que SHR pero propaga el bit de signo (relleno con
   unos si el valor es negativo), para seguir siendo equivalente a
   dividir por 2^n. Tampoco hay overflow ni carry nunca.
*/
void ejecutarSAR(MaquinaVirtual *vm) {
    int32_t valorA, n, resultado;
    int escribioOk;
 
    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
    n = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;
 
    if (n < 0) {
        printf("Error: desplazamiento negativo\n");
        vm->corriendo = 0;
        return;
    }

    if (n >= 32)
        resultado = (valorA < 0) ? -1 : 0;
    else
        resultado = valorA >> n; // shift aritmetico
 
    escribioOk = escribirOperando(vm, vm->registros[REG_OP1], resultado);
    if (!escribioOk || !vm->corriendo) { vm->corriendo = 0; return; }
 
    actualizarCC_desplazamiento(vm, resultado, 0, 0);
}

void ejecutarJMP (MaquinaVirtual *vm){ 
     int desplazamiento = leerOperando(vm, vm->registros[REG_OP1]);
     if (!vm->corriendo) return;
     vm->registros[REG_IP]= vm->registros[REG_CS] + desplazamiento;   // IP = CS + desplazamiento
}

void ejecutarJP  (MaquinaVirtual *vm){
    unsigned cc = vm->registros[REG_CC];
    int N = (cc >> BIT_N) & 1;
    int Z = (cc >> BIT_Z) & 1;

    if (N == 0 && Z == 0){
        int desplazamiento = leerOperando(vm, vm->registros[REG_OP1]);
        if (!vm->corriendo) return;
        vm->registros[REG_IP]= vm->registros[REG_CS] + desplazamiento;   // IP = CS + desplazamiento
    }
}

void ejecutarJN  (MaquinaVirtual *vm){
    unsigned int cc = vm->registros[REG_CC];
    int N = (cc >> BIT_N) & 1;

    if (N == 1){
        int desplazamiento = leerOperando(vm, vm->registros[REG_OP1]);
        if (!vm->corriendo) return;
        vm->registros[REG_IP] = vm->registros[REG_CS] + desplazamiento;
    }
 }
void ejecutarJZ  (MaquinaVirtual *vm){
     unsigned int cc = vm->registros[REG_CC];
    int Z = (cc >> BIT_Z) & 1;

    if (Z == 1){
        int desplazamiento = leerOperando(vm, vm->registros[REG_OP1]);
        if (!vm->corriendo) return;
        vm->registros[REG_IP] = vm->registros[REG_CS] + desplazamiento;
    }
}
void ejecutarJC(MaquinaVirtual *vm){
    unsigned int cc = vm->registros[REG_CC];
    int C = (cc >> BIT_C) & 1;

    if (C == 1){
        int desplazamiento = leerOperando(vm, vm->registros[REG_OP1]);
        if (!vm->corriendo) return;
        vm->registros[REG_IP] = vm->registros[REG_CS] + desplazamiento;
    }
}

void ejecutarJV(MaquinaVirtual *vm){
    unsigned int cc = vm->registros[REG_CC];
    int V = (cc >> BIT_V) & 1;

    if (V == 1){
        int desplazamiento = leerOperando(vm, vm->registros[REG_OP1]);
        if (!vm->corriendo) return;
        vm->registros[REG_IP] = vm->registros[REG_CS] + desplazamiento;
    }
}

void ejecutarJNP(MaquinaVirtual *vm){
    unsigned int cc = vm->registros[REG_CC];
    int N = (cc >> BIT_N) & 1;
    int Z = (cc >> BIT_Z) & 1;

    if (N == 1 || Z==1){
        int desplazamiento = leerOperando(vm, vm->registros[REG_OP1]);
        if (!vm->corriendo) return;
        vm->registros[REG_IP] = vm->registros[REG_CS] + desplazamiento;
    }
}

void ejecutarJNN(MaquinaVirtual *vm){
    unsigned int cc = vm->registros[REG_CC];
    int N = (cc >> BIT_N) & 1;

    if (N == 0){
        int desplazamiento = leerOperando(vm, vm->registros[REG_OP1]);
        if (!vm->corriendo) return;
        vm->registros[REG_IP] = vm->registros[REG_CS] + desplazamiento;
    }
}

void ejecutarJNZ(MaquinaVirtual *vm){
    unsigned int cc = vm->registros[REG_CC];
    int Z = (cc >> BIT_Z) & 1;

    if (Z == 0){
        int desplazamiento = leerOperando(vm, vm->registros[REG_OP1]);
        if (!vm->corriendo) return;
        vm->registros[REG_IP] = vm->registros[REG_CS] + desplazamiento;
    }
}

void ejecutarMOV (MaquinaVirtual *vm){ 

    int32_t valor = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;
    int escribioOK=escribirOperando(vm, vm->registros[REG_OP1], valor);
    if (!escribioOK || !vm->corriendo) { 
        vm->corriendo = 0;
        return; 
    }
    actualizarCC_logica(vm,valor);
}

void ejecutarCMP (MaquinaVirtual *vm){
    int32_t valorA, valorB;
    int64_t sumaConSigno;
    uint32_t menosBSinSigno;
    uint64_t sumaSinSigno;
    int32_t resultado;

    valorA = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
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

    actualizarCC_sumaResta(vm, sumaConSigno, sumaSinSigno, resultado);

}

void ejecutarLDL (MaquinaVirtual *vm){ 
    int destino=leerOperando(vm,vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
    int operandoB = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    int parteAlta=destino & (0xFFFF0000);
    int parteBaja= (operandoB & 0xFFFF);
    int resultado= parteBaja | parteAlta;

    int escribioOk = escribirOperando(vm,vm->registros[REG_OP1],resultado);
    if (!escribioOk || !vm->corriendo) { 
        vm->corriendo = 0;
        return; 
    }
}
void ejecutarLDH (MaquinaVirtual *vm){ 
    int destino=leerOperando(vm,vm->registros[REG_OP1]);
    if (!vm->corriendo) return;
    int operandoB = leerOperando(vm, vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    int parteBaja=(uint32_t)destino & 0xFFFFu;
    int parteAlta= ((uint32_t)operandoB & 0xFFFFu)<<16;
    int resultado= (int32_t)(parteBaja | parteAlta);

    
    int escribioOk = escribirOperando(vm,vm->registros[REG_OP1],resultado);
    if (!escribioOk || !vm->corriendo) { 
        vm->corriendo = 0;
        return; 
    }
}
void ejecutarRND (MaquinaVirtual *vm){ 
    int32_t resultado, op2;
    int escribioOk;

    op2=leerOperando(vm,vm->registros[REG_OP2]);
    if (!vm->corriendo) return;

    if (op2 <= 0)
        resultado=0;
    else
        resultado= rand() % ((int64_t)op2+1);

    escribioOk = escribirOperando(vm,vm->registros[REG_OP1],resultado);
    if (!escribioOk || !vm->corriendo) { 
        vm->corriendo = 0;
        return; 
    }
}

/* SYS 0 (CLEAR SCREEN): no usa ni modifica registros */
static void sysClearScreen(void){
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}

/* SYS 3 (STRING READ): lee una linea y la guarda byte a byte desde EDX, terminada en '\0'.
   ECX = cantidad maxima de caracteres (sin contar el '\0'); -1 = sin limite.
   Lo que exceda el maximo se descarta, para no dejar basura para la proxima lectura.
   Cada byte pasa por escribirMemoria, que hace el control de segmento. */
static void sysStringRead(MaquinaVirtual *vm){
    uint32_t edx = (uint32_t)vm->registros[REG_EDX];
    int32_t maximo = vm->registros[REG_ECX];
    int32_t i = 0;
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF){
        if (ch == '\r')                       // por si la entrada viene con fin de linea de Windows
            continue;
        if (maximo == -1 || i < maximo){
            if (!escribirMemoria(vm, edx + i, 1, ch)){
                limpiarBufferEntrada();       // descarta el resto de la linea
                vm->corriendo = 0;
                return;
            }
            i++;
        }
    }
    if (!escribirMemoria(vm, edx + i, 1, 0))
        vm->corriendo = 0;
}

/* SYS 4 (STRING WRITE): imprime desde EDX hasta encontrar '\0' */
static void sysStringWrite(MaquinaVirtual *vm){
    uint32_t edx = (uint32_t)vm->registros[REG_EDX];
    int32_t c = 0;
    uint32_t i;

    for (i = 0; ; i++){
        if (!leerMemoria(vm, edx + i, 1, &c)){
            fflush(stdout);
            vm->corriendo = 0;
            return;
        }
        if ((c & 0xFF) == 0)
            break;
        putchar(c & 0xFF);
    }
    fflush(stdout);
}

void ejecutarSYS(MaquinaVirtual *vm) {
    int32_t tipoOp, ecx, eax, edx, cantidad, tamano, direccionFisica, valor, i, b;
    uint32_t direccionLogica;
    unsigned char c;
    char binario[33];
    char buffer[64];
    char *finConversion;
    long valorLargo;
    int okEscritura;
    int32_t valorDecimal;
    const char *separador;
    uint32_t acumulado;
    int ch, finLinea;

    tipoOp = leerOperando(vm, vm->registros[REG_OP1]);
    if (!vm->corriendo) return;

    if (tipoOp == 0xF){
        breakpoint(vm);
        return;
    }
    if (tipoOp == 0){      // CLEAR SCREEN
        sysClearScreen();
        return;
    }
    if (tipoOp == 3){      // STRING READ
        sysStringRead(vm);
        return;
    }
    if (tipoOp == 4){      // STRING WRITE
        sysStringWrite(vm);
        return;
    }
    ecx = vm->registros[REG_ECX];
    eax = vm->registros[REG_EAX];
    edx = vm->registros[REG_EDX];
    cantidad = ecx & 0xFFFF;
    tamano   = (ecx >> 16) & 0xFFFF;

    if (tipoOp == 2)  // WRITE
        for (i = 0; i < cantidad; i++) {
            direccionLogica = edx + i * tamano;
            direccionFisica = direccionLogicaAFisica(direccionLogica, vm->segmentos, vm->memoria.tamano);
            if (direccionFisica == -1) { printf("Fallo de segmento\n"); vm->corriendo = 0; return; }

            valor = 0;
            if (!leerMemoria(vm, direccionLogica, tamano, &valor)) { vm->corriendo = 0; return; }

            // Para el decimal se extiende el signo de la celda: si la celda es de 1 o 2 bytes, su bit mas alto es el bit de signo. 

            valorDecimal = valor;
            if (tamano < 4)
                valorDecimal = (int32_t)((uint32_t)valor << (32 - 8 * tamano)) >> (32 - 8 * tamano);

            printf("[%04X]: ", direccionFisica);
            separador = "";

            if (eax & 0x10) {
                obtenerBinario((unsigned int)valor, binario);
                printf("%s0b%s", separador, binario);
                separador = " ";
            }
            if (eax & 0x08) {
                printf("%s0x%x", separador, (unsigned int)valor);
                separador = " ";
            }
            if (eax & 0x04) {
                printf("%s0o%o", separador, (unsigned int)valor);
                separador = " ";
            }
            if (eax & 0x02) {
                printf("%s", separador);
                for (b = tamano; b > 0; b--) {
                    c = (unsigned char)(valor >> (8 * (b - 1)));
                    printf("%c", (c >= 32 && c <= 126) ? c : '.');
                }
                separador = " ";
            }
            if (eax & 0x01)
                printf("%s%d", separador, valorDecimal);

            printf("\n");
        }
     else 
        if (tipoOp == 1) {  // READ
            for (i = 0; i < cantidad; i++) {
                direccionLogica = edx + i * tamano;
                direccionFisica = direccionLogicaAFisica(direccionLogica, vm->segmentos, vm->memoria.tamano);
                if (direccionFisica == -1) { printf("Fallo de segmento\n"); vm->corriendo = 0; return; }
    
                printf("[%04X]: ", direccionFisica);
    
                if (eax == 0x02) {
                    /* Caracteres: se leen tamano bytes sueltos y se
                    empaquetan en un solo valor, MSB primero (igual
                    orden que desempaqueta el WRITE). */
                    acumulado = 0;
                    finLinea = 0;
                    for (b = 0; b < tamano; b++) {
                        ch = finLinea ? 0 : getchar();
                        if (ch == '\n' || ch == EOF) {   // consume el enter 
                            finLinea = 1;              
                            ch = 0;
                        }
                        acumulado = (acumulado << 8) | (unsigned char)ch;
                    }
                    if (!finLinea)
                        limpiarBufferEntrada();        /* descarta lo que sobre de la linea */
                    valor = (int32_t)acumulado;
                } else {
                    /* Decimal (0x01), octal (0x04), hex (0x08) o
                    binario (0x10): base distinta segun el bit. */
                    int base;
                    if (eax == 0x01) 
                        base = 10;
                    else 
                        if (eax == 0x04) 
                            base = 8;
                        else 
                            if (eax == 0x08) 
                                base = 16;
                            else    //0x10                
                                base = 2;   
    
                    if (scanf("%63s", buffer) != 1){
                        printf("Error al leer la entrada\n");
                        vm->corriendo = 0;
                        return;
                    }
    
                    /* Acepta los prefijos 0b y 0o (strtol solo entiende 0x):
                       se saltea el signo y, si esta el prefijo, se lo borra. */
                    {
                        char *digitos = buffer;
                        if (*digitos == '-' || *digitos == '+')
                            digitos++;
                        if (digitos[0] == '0' &&
                            ((base == 2 && (digitos[1] == 'b' || digitos[1] == 'B')) ||
                             (base == 8 && (digitos[1] == 'o' || digitos[1] == 'O'))))
                            memmove(digitos, digitos + 2, strlen(digitos + 2) + 1);
                    }

                    errno = 0;
                    valorLargo = strtol(buffer, &finConversion, base);

                    if (finConversion == buffer || *finConversion != '\0') {
                        printf("Valor invalido, reingrese\n");
                        limpiarBufferEntrada();
                        i--;          /* repite la misma celda */
                        continue;
                    }
    
                    /* Ancho + comparacion, mismo patron que en ADD/MUL/SHL:
                    strtol devuelve long (ancho no garantizado), asi que
                    hay que chequear el rango de 32 bits a mano ademas
                    de errno. */
                    if (errno == ERANGE || valorLargo > INT32_MAX || valorLargo < INT32_MIN) {
                        printf("Advertencia: valor leido fuera de rango, se trunca\n");
                    }
                    valor = (int32_t)valorLargo;
    
                    limpiarBufferEntrada();
                }
    
                okEscritura = escribirMemoria(vm, direccionLogica, tamano, valor);
                if (!okEscritura || !vm->corriendo) { vm->corriendo = 0; return; }
            }
        }
}
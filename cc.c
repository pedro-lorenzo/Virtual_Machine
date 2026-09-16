#include <limits.h>
#include "cc.h"
#include "Registros.h"
#define BIT_N 31
#define BIT_Z 30
#define BIT_C 29
#define BIT_V 28

static void setCC(MaquinaVirtual *vm, int N, int Z, int C, int V) {
    vm->registros[REG_CC] =
          ((unsigned int)(N & 1) << BIT_N)
        | ((unsigned int)(Z & 1) << BIT_Z)
        | ((unsigned int)(C & 1) << BIT_C)
        | ((unsigned int)(V & 1) << BIT_V);
}

void actualizarCC_sumaResta(MaquinaVirtual *vm, int64_t sumaConSigno, uint64_t sumaSinSigno, int32_t resultado) {
    int N = (resultado < 0) ? 1 : 0;
    int Z = (resultado == 0) ? 1 : 0;
    int C = (sumaSinSigno > 0xFFFFFFFFULL) ? 1 : 0;
    int V = (sumaConSigno > INT32_MAX || sumaConSigno < INT32_MIN) ? 1 : 0;
    setCC(vm, N, Z, C, V);
}

void actualizarCC_producto(MaquinaVirtual *vm, int64_t productoConSigno, uint64_t productoSinSigno, int32_t resultado) {
    int N = (resultado < 0) ? 1 : 0;
    int Z = (resultado == 0) ? 1 : 0;
    int C = (productoSinSigno > 0xFFFFFFFFULL) ? 1 : 0;
    int V = (productoConSigno > INT32_MAX || productoConSigno < INT32_MIN) ? 1 : 0;
    setCC(vm, N, Z, C, V);
}

void actualizarCC_division(MaquinaVirtual *vm, int32_t resultado, int overflowDetectado) {
    int N = (resultado < 0) ? 1 : 0;
    int Z = (resultado == 0) ? 1 : 0;
    setCC(vm, N, Z, /* C */ 0, /* V */ overflowDetectado ? 1 : 0);
}

void actualizarCC_logica(MaquinaVirtual *vm, int32_t resultado) {
    int N = (resultado < 0) ? 1 : 0;
    int Z = (resultado == 0) ? 1 : 0;
    setCC(vm, N, Z, /* C */ 0, /* V */ 0);
}
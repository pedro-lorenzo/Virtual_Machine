#include <stdint.h>
#include "cc.h"
#include "Registros.h"

static void setCC(MaquinaVirtual *vm, int32_t N, int32_t Z, int32_t C, int32_t V) {
    vm->registros[REG_CC] = (N & 1) ? INT32_MIN : 0;

    vm->registros[REG_CC] |=
          ((Z & 1) << BIT_Z)
        | ((C & 1) << BIT_C)
        | ((V & 1) << BIT_V);
}

void actualizarCC_sumaResta(MaquinaVirtual *vm, int64_t sumaConSigno, uint64_t sumaSinSigno, int32_t resultado) {
    int32_t N = (resultado < 0) ? 1 : 0;
    int32_t Z = (resultado == 0) ? 1 : 0;
    int32_t C = (sumaSinSigno > 0xFFFFFFFFULL) ? 1 : 0;
    int32_t V = (sumaConSigno > INT32_MAX || sumaConSigno < INT32_MIN) ? 1 : 0;
    setCC(vm, N, Z, C, V);
}

void actualizarCC_producto(MaquinaVirtual *vm, int64_t productoConSigno, uint64_t productoSinSigno, int32_t resultado) {
    int32_t N = (resultado < 0) ? 1 : 0;
    int32_t Z = (resultado == 0) ? 1 : 0;
    int32_t C = (productoSinSigno > 0xFFFFFFFFULL) ? 1 : 0;
    int32_t V = (productoConSigno > INT32_MAX || productoConSigno < INT32_MIN) ? 1 : 0;
    setCC(vm, N, Z, C, V);
}

void actualizarCC_division(MaquinaVirtual *vm, int32_t resultado, int32_t overflowDetectado) {
    int32_t N = (resultado < 0) ? 1 : 0;
    int32_t Z = (resultado == 0) ? 1 : 0;
    setCC(vm, N, Z, /* C */ 0, /* V */ overflowDetectado ? 1 : 0);
}

void actualizarCC_logica(MaquinaVirtual *vm, int32_t resultado) {
    int32_t N = (resultado < 0) ? 1 : 0;
    int32_t Z = (resultado == 0) ? 1 : 0;
    setCC(vm, N, Z, /* C */ 0, /* V */ 0);
}

void actualizarCC_desplazamiento(MaquinaVirtual *vm, int32_t resultado, int C, int V) {
    int N = (resultado < 0) ? 1 : 0;
    int Z = (resultado == 0) ? 1 : 0;
    setCC(vm, N, Z, C ? 1 : 0, V ? 1 : 0);
}
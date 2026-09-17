#ifndef CC_H
#define CC_H

#include <stdint.h>
#include "tipos.h"


void actualizarCC_sumaResta(MaquinaVirtual *vm, int64_t sumaConSigno, uint64_t sumaSinSigno, int32_t resultado);

void actualizarCC_producto(MaquinaVirtual *vm, int64_t productoConSigno, uint64_t productoSinSigno, int32_t resultado);

void actualizarCC_division(MaquinaVirtual *vm, int32_t resultado, int32_t overflowDetectado);

void actualizarCC_logica(MaquinaVirtual *vm, int32_t resultado);

void actualizarCC_desplazamiento(MaquinaVirtual *vm, int32_t resultado, int C, int V);

#endif
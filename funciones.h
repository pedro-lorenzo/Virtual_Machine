#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <stdint.h>
#include "tipos.h"
int32_t leerOperando(MaquinaVirtual *vm, uint32_t registroOP);
void iniciaMaquinaVirtual(Cabecera cabecera, tSegmento segmentos, tRegistro registros);
int32_t cantidadOperandosALeer(int32_t codigo);
int32_t direccionLogicaAFisica(uint32_t direccionLogica, tSegmento segmentos);
void procesaPrograma(MaquinaVirtual *vm);
void cargarLAR(MaquinaVirtual *vm, uint32_t direccionLogica);
void cargarCantidadBytesMAR(MaquinaVirtual *vm, uint32_t cantidadBytes);
void cargarDireccionMAR(MaquinaVirtual *vm, int32_t direccionFisica);
int32_t leerMemoria(MaquinaVirtual *vm, uint32_t direccionLogica, uint32_t cantidadBytes, int32_t *resultado);
int32_t PrepararAccesoMemoria(MaquinaVirtual *vm, uint32_t direccionLogica, uint32_t cantidadBytes);
int32_t escribirOperando(MaquinaVirtual *vm, uint32_t registroOP, int32_t valor);
int32_t escribirMemoria(MaquinaVirtual *vm, uint32_t direccionLogica, uint32_t cantidadBytes, int32_t valor);
void obtenerBinario(unsigned int valor, char *binario);
#endif

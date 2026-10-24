#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <stdint.h>
#include "tipos.h"
int32_t leerOperando(MaquinaVirtual *vm, uint32_t registroOP);
uint32_t sectorRegistro(uint32_t operando);       // 0=EAX 1=AL 2=AH 3=AX (sirve tambien para el disassembler)
uint32_t bytesModificador(uint32_t operando);     // 1 (b), 2 (w) o 4 (l / sin prefijo)
int32_t extenderSigno(uint32_t valor, uint32_t cantidadBytes);
uint32_t direccionOperandoMemoria(MaquinaVirtual *vm, uint32_t operando);
int32_t iniciaMaquinaVirtual(MaquinaVirtual *vm, Cabecera cabecera);
int32_t cantidadOperandosALeer(int32_t codigo);
int32_t direccionLogicaAFisica(uint32_t direccionLogica, tSegmento segmentos, uint32_t tamMemoria);
void procesaPrograma(MaquinaVirtual *vm);
void cargarLAR(MaquinaVirtual *vm, uint32_t direccionLogica);
void cargarCantidadBytesMAR(MaquinaVirtual *vm, uint32_t cantidadBytes);
void cargarDireccionMAR(MaquinaVirtual *vm, int32_t direccionFisica);
int32_t leerMemoria(MaquinaVirtual *vm, uint32_t direccionLogica, uint32_t cantidadBytes, int32_t *resultado);
int32_t PrepararAccesoMemoria(MaquinaVirtual *vm, uint32_t direccionLogica, uint32_t cantidadBytes);
int32_t escribirOperando(MaquinaVirtual *vm, uint32_t registroOP, int32_t valor);
int32_t escribirMemoria(MaquinaVirtual *vm, uint32_t direccionLogica, uint32_t cantidadBytes, int32_t valor);
void obtenerBinario(unsigned int valor, char *binario);
void limpiarBufferEntrada();
int32_t crearMemoria(Memoria *memoria, uint32_t kib);
void liberarMemoria(Memoria *memoria);
#endif

#ifndef FUNCIONES_H
#define FUNCIONES_H

#include "tipos.h"
int leerOperando(MaquinaVirtual *vm, unsigned int registroOP);
void iniciaMaquinaVirtual(Cabecera cabecera, tSegmento segmentos, tRegistro registros);
int cantidadOperandosALeer(int codigo);
int direccionLogicaAFisica(unsigned int direccionLogica, tSegmento segmentos);
void procesaPrograma(MaquinaVirtual *vm);
void cargarLAR(MaquinaVirtual *vm, unsigned int direccionLogica);
void cargarCantidadBytesMAR(MaquinaVirtual *vm, unsigned int cantidadBytes);
void cargarDireccionMAR(MaquinaVirtual *vm, int direccionFisica);
int leerMemoria(MaquinaVirtual *vm, unsigned int direccionLogica, unsigned int cantidadBytes, int *resultado);
int PrepararAccesoMemoria(MaquinaVirtual *vm, unsigned int direccionLogica, unsigned int cantidadBytes);
int escribirOperando(MaquinaVirtual *vm, unsigned int registroOP, int valor);
int escribirMemoria(MaquinaVirtual *vm, unsigned int direccionLogica, unsigned int cantidadBytes, int valor);

#endif

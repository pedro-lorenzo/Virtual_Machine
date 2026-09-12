#ifndef FUNCIONES_H
#define FUNCIONES_H

#include "tipos.h"

void iniciaMaquinaVirtual(Cabecera cabecera, tSegmento segmentos, tRegistro registros);
int cantidadOperandosALeer(int codigo);
int direccionLogicaAFisica(unsigned int direccionLogica,
                           tSegmento segmentos);
void procesaPrograma(MaquinaVirtual *vm);

#endif

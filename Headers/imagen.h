#ifndef IMAGEN_H
#define IMAGEN_H

#include <stdint.h>
#include "tipos.h"

int32_t guardarImagen(MaquinaVirtual *vm, const char *nombreArchivo);
int32_t cargarImagen(MaquinaVirtual *vm, const char *nombreArchivo);
void breakpoint(MaquinaVirtual *vm);

#endif
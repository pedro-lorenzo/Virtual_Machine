
#define INSTRUCCIONES_H

#include "tipos.h"

/* Todas las funciones de instruccion comparten esta firma, sin importar
   cuantos operandos usen realmente. Las que no necesitan OP2 (o ninguno
   de los dos) simplemente no lo miran. */
typedef void (*InstruccionFn)(MaquinaVirtual *vm);

/* ---- Un operando ---- */
void ejecutarSYS(MaquinaVirtual *vm);
void ejecutarJMP(MaquinaVirtual *vm);
void ejecutarJP (MaquinaVirtual *vm);
void ejecutarJN (MaquinaVirtual *vm);
void ejecutarJZ (MaquinaVirtual *vm);
void ejecutarJC (MaquinaVirtual *vm);
void ejecutarJV (MaquinaVirtual *vm);
void ejecutarJNP(MaquinaVirtual *vm);
void ejecutarJNN(MaquinaVirtual *vm);
void ejecutarJNZ(MaquinaVirtual *vm);
void ejecutarNOT(MaquinaVirtual *vm);

/* ---- Dos operandos ---- */
void ejecutarMOV (MaquinaVirtual *vm);
void ejecutarADD (MaquinaVirtual *vm);
void ejecutarSUB (MaquinaVirtual *vm);
void ejecutarMUL (MaquinaVirtual *vm);
void ejecutarDIV (MaquinaVirtual *vm);
void ejecutarCMP (MaquinaVirtual *vm);
void ejecutarAND (MaquinaVirtual *vm);
void ejecutarOR  (MaquinaVirtual *vm);
void ejecutarXOR (MaquinaVirtual *vm);
void ejecutarSWAP(MaquinaVirtual *vm);
void ejecutarSHL (MaquinaVirtual *vm);
void ejecutarSHR (MaquinaVirtual *vm);
void ejecutarSAR (MaquinaVirtual *vm);
void ejecutarLDL (MaquinaVirtual *vm);
void ejecutarLDH (MaquinaVirtual *vm);
void ejecutarRND (MaquinaVirtual *vm);

/* Codigo de operacion que no existe (huecos de la tabla: 0x0B-0x0E,
   o cualquier valor fuera de 0-31 si algun dia llegara a pasar) */
void instruccionInvalida(MaquinaVirtual *vm);


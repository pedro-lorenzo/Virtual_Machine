#include "Registros.h"
#include "funciones.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int32_t crearMemoria(Memoria *memoria, uint32_t kib){
    memoria->tamano = kib * 1024;
    memoria->datos = calloc(memoria->tamano, sizeof(uint8_t));
    if (memoria->datos == NULL){
        memoria->tamano = 0;
        return -1;
    }
    return 0;
}

void liberarMemoria(Memoria *memoria){
    free(memoria->datos);
    memoria->datos = NULL;
    memoria->tamano = 0;
}

void iniciaMaquinaVirtual(Cabecera cabecera, tSegmento segmentos, tRegistro registros, uint32_t tamMemoria){
    int32_t i;

    segmentos[0] = cabecera.tamano;                                 //Inicia el segmento de código
    segmentos[1] = cabecera.tamano;
    segmentos[1]= segmentos[1] << 16;
    segmentos[1] = segmentos[1] | (tamMemoria - cabecera.tamano);    //Inicia parte alta y baja del Segmento de datos

    for (i=2; i<CANT_SEGMENTOS; i++)
        segmentos[i] = -1;

    registros[REG_CS] = 0x00000000;  //Inicializacion CS
    registros[REG_DS] = 0x00010000; //Inicializacion DS
    registros[REG_ES] = -1;          //Segmentos inexistentes (0xFFFFFFFF)
    registros[REG_SS] = -1;
    registros[REG_KS] = -1;
    registros[REG_PS] = -1;
    registros[REG_IP] = registros[REG_CS]; //Inicializamos IP con CS

    for (i=REG_EAX; i<=REG_EFX; i++)  //inicia los de proposito general
        registros[i] = 0;
}
int32_t cantidadOperandosALeer(int32_t codigo) {
    if (codigo == 0x0E || codigo == 0x0F)   // RET y STOP no tienen operandos
        return 0;
    else
        return ((codigo & 0xF0) >> 4) + 1;
}

int32_t direccionLogicaAFisica(uint32_t direccionLogica, tSegmento segmentos, uint32_t tamMemoria) {
    uint32_t codigoSegmento;
    uint32_t desplazamiento;
    int32_t segmento;
    uint32_t base;
    uint32_t tamano;

    codigoSegmento = direccionLogica >> 16;
    desplazamiento = direccionLogica & 0xFFFF;

    if (codigoSegmento >= CANT_SEGMENTOS) {
        return -1;
    }

    segmento = segmentos[codigoSegmento];

    if (segmento == -1) {
        return -1;
    }

    base = ((uint32_t)segmento) >> 16;
    tamano = ((uint32_t)segmento) & 0xFFFF;

    if (base > tamMemoria)
        return -1;

    if (tamano > tamMemoria - base)
        return -1;

    if (desplazamiento >= tamano) {
        return -1;
    }

    return (int32_t)(base + desplazamiento);
}

/* ---- Sectores de registro y modificadores de memoria ----
   Operando de registro (1 byte):  X | 2 bits sector | 5 bits codigo
       sector 00 = EAX (4 bytes), 01 = AL (4to byte), 10 = AH (3er byte), 11 = AX (2 bytes)
   Operando de memoria (3 bytes):  16 bits offset | 3 bits tamano | 5 bits codigo
       tamano 001 = b (1 byte), 010 = w (2 bytes), 100 = l (4 bytes), 000 = sin prefijo (4 bytes) */

static const uint32_t MASCARA_SECTOR[4]    = {0xFFFFFFFFu, 0x000000FFu, 0x0000FF00u, 0x0000FFFFu};
static const uint32_t CORRIMIENTO_SECTOR[4] = {0, 0, 8, 0};

uint32_t sectorRegistro(uint32_t operando){
    return (operando >> 5) & 0x3;
}

uint32_t bytesModificador(uint32_t operando){
    uint32_t modificador = (operando >> 5) & 0x7;
    if (modificador == 1) return 1;
    if (modificador == 2) return 2;
    return 4;                       // 100 (l) o 000 (sin prefijo / programas v1)
}

/* Extiende el signo de un valor de n bytes a 32 bits */
int32_t extenderSigno(uint32_t valor, uint32_t cantidadBytes){
    uint32_t corrimiento;
    if (cantidadBytes >= 4)
        return (int32_t)valor;
    corrimiento = 32 - 8 * cantidadBytes;
    return (int32_t)(valor << corrimiento) >> corrimiento;
}

/* Arma la direccion logica de un operando de memoria: contenido del registro + offset con signo */
uint32_t direccionOperandoMemoria(MaquinaVirtual *vm, uint32_t operando){
    uint32_t codigoRegistro = operando & 0x1F;
    int32_t desplazamiento = extenderSigno((operando >> 8) & 0xFFFFu, 2);
    return (uint32_t)vm->registros[codigoRegistro] + desplazamiento;
}

int32_t leerOperando(MaquinaVirtual *vm, uint32_t registroOP){
    int32_t valor;
    uint32_t tipo, operando, sector, contenido, cantidadBytes;

    tipo = (registroOP >> 24 )& 0xFF;
    operando = registroOP & 0xFFFFFF; //valor operando codificado

    if (tipo == 1 ){        //Registro
        sector = sectorRegistro(operando);
        contenido = (uint32_t)vm->registros[operando & 0x1F];
        contenido = (contenido & MASCARA_SECTOR[sector]) >> CORRIMIENTO_SECTOR[sector];
        if (sector == 3)
            return extenderSigno(contenido, 2);   // AX
        if (sector != 0)
            return extenderSigno(contenido, 1);   // AL o AH
        return (int32_t)contenido;                // EAX
    }
    else
        if (tipo == 2){       //Inmediato
            return extenderSigno(operando & 0xFFFFu, 2);
        }
        else if (tipo==3){                  //Memoria
            cantidadBytes = bytesModificador(operando);
            valor = 0;

            if (!leerMemoria(vm, direccionOperandoMemoria(vm, operando), cantidadBytes, &valor))
                return 0;

            return extenderSigno((uint32_t)valor, cantidadBytes);
        } else{
            printf("Tipo de operando invalido\n");
            vm->corriendo = 0;
            return 0;
        }
}

void cargarLAR(MaquinaVirtual *vm, uint32_t direccionLogica){
    vm->registros[REG_LAR] = direccionLogica;
}

void cargarCantidadBytesMAR(MaquinaVirtual *vm, uint32_t cantidadBytes){
    vm->registros[REG_MAR] = (cantidadBytes & 0xFFFF) << 16;
}

void cargarDireccionMAR(MaquinaVirtual *vm, int32_t direccionFisica){
    vm->registros[REG_MAR] |= (direccionFisica & 0xFFFFu);
}

int32_t leerMemoria(MaquinaVirtual *vm, uint32_t direccionLogica, uint32_t cantidadBytes, int32_t *resultado){
    int32_t direccionFisica = PrepararAccesoMemoria(vm, direccionLogica, cantidadBytes);

    if (direccionFisica == -1)
        return 0;
    else{
        uint32_t valor = 0;
        for (uint32_t i=0; i < cantidadBytes; i++){
            valor = valor << 8;
            valor = valor | vm->memoria.datos[direccionFisica + i];
        }
        *resultado = (int32_t)valor;

        vm->registros[REG_MBR] = *resultado; // carga MBR

        return 1;
    }
}

int32_t PrepararAccesoMemoria(MaquinaVirtual *vm, uint32_t direccionLogica, uint32_t cantidadBytes){
    cargarLAR(vm, direccionLogica);
    cargarCantidadBytesMAR(vm, cantidadBytes);

    int32_t direccionFisica = direccionLogicaAFisica(direccionLogica, vm->segmentos, vm->memoria.tamano);

    if (direccionFisica == -1){
        vm->corriendo = 0;
        printf("Fallo de segmento\n");
        return -1;
    }

    uint32_t codigoSegmento = direccionLogica >> 16;
    uint32_t desplazamiento = direccionLogica & 0xFFFF;
    uint32_t tamanoSegmento = (uint32_t)vm->segmentos[codigoSegmento] & 0xFFFF;

    if (cantidadBytes == 0 || cantidadBytes > tamanoSegmento || desplazamiento > tamanoSegmento - cantidadBytes || cantidadBytes > 4){
        vm->corriendo = 0;
        printf("Fallo de segmento\n");
        return -1;
    }

    cargarDireccionMAR(vm, direccionFisica);

    return direccionFisica;
}

int32_t escribirOperando (MaquinaVirtual *vm, uint32_t registroOP, int32_t valor){ //devuelve 1 si escribio o 0 si no
    uint32_t tipo = (registroOP >> 24) & 0xFFu;

    uint32_t operando = registroOP & 0xFFFFFFu;

    if (tipo == 1){
        // Reemplaza solo los bits del sector; el resto del registro queda igual (valor truncado)
        uint32_t codigoRegistro = operando & 0x1F;
        uint32_t sector = sectorRegistro(operando);
        uint32_t mascara = MASCARA_SECTOR[sector];
        uint32_t actual = (uint32_t)vm->registros[codigoRegistro];
        uint32_t nuevo = ((uint32_t)valor << CORRIMIENTO_SECTOR[sector]) & mascara;
        vm->registros[codigoRegistro] = (int32_t)((actual & ~mascara) | nuevo);
        return 1;
    }
    else{
        if (tipo == 3){
            // escribirMemoria guarda los cantidadBytes menos significativos (trunca)
            return escribirMemoria(vm, direccionOperandoMemoria(vm, operando), bytesModificador(operando), valor);
        }
        //sino operando destino invalido
        return 0;
    }
}

int32_t escribirMemoria (MaquinaVirtual *vm, uint32_t direccionLogica, uint32_t cantidadBytes, int32_t valor){
    int32_t direccionFisica = PrepararAccesoMemoria(vm, direccionLogica, cantidadBytes);

    if (direccionFisica == -1)
        return 0;

    vm->registros[REG_MBR] = valor;

    uint32_t valorAuxiliar = valor;

    for (uint32_t i = cantidadBytes; i > 0; i--) {
        vm->memoria.datos[direccionFisica + i - 1] = (uint8_t)(valorAuxiliar & 0xFFu);
        valorAuxiliar >>= 8;
    }

    return 1;
}
void obtenerBinario(unsigned int valor, char *binario){
    char temp[33];
    int i = 32;
    temp[32] = '\0';   // el buffer termina con el caracter nulo

    if (valor == 0){
        binario[0] = '0';
        binario[1] = '\0';
        return;
    }

    while (valor > 0){
        i--;
        temp[i] = (valor % 2) + '0';   // resto de dividir por 2 = el bit actual, como caracter '0' o '1'
        valor /= 2;                     // "corro" el numero para la siguiente vuelta
    }

    strcpy(binario, &temp[i]);   // copio desde donde empezaron los digitos reales
}
void limpiarBufferEntrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ; //descarta
}
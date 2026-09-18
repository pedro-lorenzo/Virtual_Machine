#include "Registros.h"
#include "funciones.h"
#include <stdio.h>
#include <string.h>

void iniciaMaquinaVirtual(Cabecera cabecera, tSegmento segmentos, tRegistro registros){
    int32_t i;

    segmentos[0] = cabecera.tamano;                                 //Inicia el segmento de código
    segmentos[1] = cabecera.tamano;
    segmentos[1]= segmentos[1] << 16;
    segmentos[1] = segmentos[1] | TAM_MEMORIA - cabecera.tamano;    //Inicia parte alta y baja del Segmento de datos

    for (i=2; i<CANT_SEGMENTOS; i++)
        segmentos[i] = -1;

    registros[26] = 0x00000000;  //Inicializacion CS
    registros[27] = 0x00010000; //Inicializacion DS
    registros[0] = registros[26]; //Inicializamos IP con CS

    for (i=10; i<16; i++)  //inicia los de proposito general
        registros[i] = 0;
}
int32_t cantidadOperandosALeer(int32_t codigo) {
    if (codigo == 0x0F)
        return 0;
    else
        return ((codigo & 0xF0) >> 4) + 1;
}

int32_t direccionLogicaAFisica(uint32_t direccionLogica, tSegmento segmentos) {
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

    if (desplazamiento >= tamano) {
        return -1;
    }

    return (int32_t)(base + desplazamiento);
}

int32_t leerOperando(MaquinaVirtual *vm, uint32_t registroOP){
    int32_t desplazamiento, valor;
    uint32_t direccionLogica, codigoRegistro, tipo, operando;

    tipo = (registroOP >> 24 )& 0xFF;
    operando = registroOP & 0xFFFFFF; //valor operando codificado
    
    if (tipo == 1 )         //Registro
        return vm->registros[operando & 0x1F];
    else
        if (tipo == 2){       //Inmediato
            valor = (int32_t)((operando & 0xFFFFu) << 16);
            valor = valor >> 16;
            return valor;
        }
        else{                  //Memoria
            codigoRegistro = operando & 0x1F;  // 5 bits de mas abajo
            desplazamiento =(operando >> 8) & 0xFFFF; // los 16 bits de arriba
        
            //por si arrastra el signo (igual que en funcion direccion logica a fisica)
            desplazamiento = desplazamiento << 16;
            desplazamiento = desplazamiento >> 16;
        
            direccionLogica = (uint32_t)vm ->registros[codigoRegistro] + desplazamiento; // construye la direccion logica: segmento(cont del reg) y offset(desplazamiento)
        
            //lee los 4 bytes empezando desde la direccion fisica
            valor = 0;

            if (!leerMemoria(vm, direccionLogica,4,&valor))
                return 0;

            return valor;
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
        for (int32_t i=0; i < cantidadBytes; i++){
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

    int32_t direccionFisica = direccionLogicaAFisica(direccionLogica, vm->segmentos);

    if (direccionFisica == -1){
        vm->corriendo = 0;
        return -1;
    }

    uint32_t codigoSegmento = direccionLogica >> 16;
    uint32_t desplazamiento = direccionLogica & 0xFFFF;
    uint32_t tamanoSegmento = (uint32_t)vm->segmentos[codigoSegmento] & 0xFFFF;

    if (cantidadBytes == 0 || cantidadBytes > tamanoSegmento || desplazamiento > tamanoSegmento - cantidadBytes || cantidadBytes > 4){
        vm->corriendo = 0;
        return -1;
    }

    cargarDireccionMAR(vm, direccionFisica);

    return direccionFisica;
}

int32_t escribirOperando (MaquinaVirtual *vm, uint32_t registroOP, int32_t valor){ //devuelve 1 si escribio o 0 si no
    uint32_t tipo = (registroOP >> 24) & 0xFFu;

    uint32_t operando = registroOP & 0xFFFFFFu;

    if (tipo == 1){
        uint32_t codigoRegistro = operando & 0x1F;
        vm->registros[codigoRegistro] = valor;
        return 1;
    }
    else{
        if (tipo == 3){
            uint32_t codigoRegistro = operando & 0x1F;
            int32_t desplazamiento = (operando >> 8) & 0xFFFF;
            desplazamiento = desplazamiento << 16;
            desplazamiento = desplazamiento >> 16;
            uint32_t direccionLogica = (uint32_t)vm->registros[codigoRegistro] + desplazamiento;
            return escribirMemoria(vm, direccionLogica, 4, valor);
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
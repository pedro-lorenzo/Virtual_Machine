#include "tipos.h"
#include "funciones.h"
#include <stdio.h>


void iniciaMaquinaVirtual(Cabecera cabecera, tSegmento segmentos, tRegistro registros){
    int i;

    segmentos[0] = cabecera.tamano;                                 //Inicia el segmento de código
    segmentos[1] = cabecera.tamano;
    segmentos[1]= segmentos[1] << 16;
    segmentos[1] = segmentos[1] | TAM_MEMORIA - cabecera.tamano;    //Inicia parte alta y baja del Segmento de datos

    for (i=2; i<CANT_SEGMENTOS; i++)
        segmentos[i] = -1;

    registros[26] = 0x00000000;  //Inicializacion CS
    registros[27] = 0x00010000; //Inicializacion DS
    registros[0] = registros[26]; //Inicializamos IP con CS
}


int cantidadOperandosALeer(int codigo) {
    if (codigo == 0x0F) 
        return 0;
    else
        return ((codigo & 0xF0) >> 4) + 1;
}

int direccionLogicaAFisica(unsigned int direccionLogica, tSegmento segmentos) {
    unsigned int codigoSegmento;
    unsigned int desplazamiento;
    int segmento;
    unsigned int base;
    unsigned int tamano;

    codigoSegmento = direccionLogica >> 16;
    desplazamiento = direccionLogica & 0xFFFF;
    desplazamiento = desplazamiento << 16;
    desplazamiento = desplazamiento >> 16;

    if (desplazamiento < 0){
        return -1;
    }

    if (codigoSegmento >= CANT_SEGMENTOS) {
        return -1;
    }

    segmento = segmentos[codigoSegmento];

    if (segmento == -1) {
        return -1;
    }

    base = ((unsigned int)segmento) >> 16;
    tamano = ((unsigned int)segmento) & 0xFFFF;

    if (desplazamiento >= tamano) {
        return -1;
    }

    return (int)(base + desplazamiento);
}

int leerOperando(MaquinaVirtual *vm, unsigned int registroOP){
    int direccionLogica, direccionFisica, codigoRegistro, codigoDesplazamiento, tipo, operando, desplazamiento, valor;

    tipo = (registroOP >> 24 )& 0xFF;
    operando = registroOP & 0xFFFFFF; //valor operando codificado
    if (tipo == 1 )         //Registro
        return vm->registros[operando & 0x1F];
    else
     if (tipo == 2)         //Inmediato
        return operando;
     else{                  //Memoria
        codigoRegistro = operando & 0x1F;  // 5 bits de mas abajo
        desplazamiento =(operando >> 8) & 0xFFFF; // los 16 bits de arriba
        
        //por si arrastra el signo (igual que en funcion direccion logica a fisica)
        desplazamiento = desplazamiento << 16;
        desplazamiento = desplazamiento >> 16;
        
        direccionLogica = (vm ->registros[codigoRegistro] << 16) | desplazamiento; // construye la direccion logica: segmento(cont del reg) y offset(desplazamiento)
        direccionFisica = direccionLogicaAFisica(direccionLogica,vm->segmentos);
        
        //lee los 4 bytes empezando desde la direccion fisica
        valor = 0;

        if (!leerMemoria(vm, direccionLogica,4,&valor))
            return 0;

        return valor;
    }
}

void cargarLAR(MaquinaVirtual *vm, unsigned int direccionLogica){
    vm->registros[REG_LAR] = direccionLogica;
}

void cargarCantidadBytesMAR(MaquinaVirtual *vm, unsigned int cantidadBytes){
    vm->registros[REG_MAR] = (cantidadBytes & 0xFFFF) << 16;
}

void cargarDireccionMAR(MaquinaVirtual *vm, int direccionFisica){
    vm->registros[REG_MAR] |= (direccionFisica & 0xFFFFu);
}

int leerMemoria(MaquinaVirtual *vm, unsigned int direccionLogica, unsigned int cantidadBytes, int *resultado){
    int direccionFisica = PrepararAccesoMemoria(vm, direccionLogica, cantidadBytes);

    if (direccionFisica == -1)
        return 0;
    else{
        int valor = 0;
        for (int i=0; i < cantidadBytes; i++){
            valor = valor << 8;
            valor = valor | vm->memoria.datos[direccionFisica + i];
        }

        vm->registros[REG_MBR] = valor; // carga MBR

        *resultado = valor;

        return 1;
    }
}

int PrepararAccesoMemoria(MaquinaVirtual *vm, unsigned int direccionLogica, unsigned int cantidadBytes){
    cargarLAR(vm, direccionLogica);
    cargarCantidadBytesMAR(vm, cantidadBytes);

    int direccionFisica = direccionLogicaAFisica(direccionLogica, vm->segmentos);

    if (direccionFisica == -1){
        vm->corriendo = 0;
        return -1;
    }

    unsigned int codigoSegmento = direccionLogica >> 16;
    unsigned int desplazamiento = direccionLogica & 0xFFFF;
    int tamanoSegmento = vm->segmentos[codigoSegmento] & 0xFFFF;

    if (cantidadBytes == 0 || cantidadBytes > tamanoSegmento || desplazamiento > tamanoSegmento - cantidadBytes){
        vm->corriendo = 0;
        return -1;
    }

    cargarDireccionMAR(vm, direccionFisica);

    return direccionFisica;
}

int escribirOperando (MaquinaVirtual *vm, unsigned int registroOP, int valor){ //devuelve 1 si escribio o 0 si no
    unsigned int tipo = (registroOP >> 24) & 0xFFu;

    unsigned int operando = registroOP & 0xFFFFFFu;

    if (tipo == 1){
        unsigned int codigoRegistro = operando & 0x1F;
        vm->registros[codigoRegistro] = valor;
        return 1;
    }
    else{
        if (tipo == 3){
            unsigned int codigoRegistro = operando & 0x1F;
            int desplazamiento = (operando >> 8) & 0xFFFF;
            desplazamiento = desplazamiento << 16;
            desplazamiento = desplazamiento >> 16;
            unsigned int direccionLogica = vm->registros[codigoRegistro] + desplazamiento;
            return escribirMemoria(vm, direccionLogica, 4, valor);
        }
        //sino operando destino invalido
        return 0;
    }
}

int escribirMemoria (MaquinaVirtual *vm, unsigned int direccionLogica, unsigned int cantidadBytes, int valor){
    int direccionFisica = PrepararAccesoMemoria(vm, direccionLogica, cantidadBytes);
    
    if (direccionFisica == -1)
        return 0;

    vm->registros[REG_MBR] = valor;

    unsigned int valorAuxiliar = valor;

    for (unsigned int i = cantidadBytes; i > 0; i--) {
        vm->memoria.datos[direccionFisica + i - 1] = (unsigned char)(valorAuxiliar & 0xFFu);
        valorAuxiliar >>= 8;
    }

    return 1;
}
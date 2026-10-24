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

/* Arma la tabla de descriptores y los registros de segmento (tabla dinamica).
   Recorre los segmentos en el orden en que van en la memoria (SEG_PARAM .. SEG_STACK):
   cada segmento con tamano > 0 se ubica a continuacion del anterior y ocupa la
   siguiente entrada libre de la tabla. Los que miden 0 no tienen entrada y su
   registro queda en -1.
     tabla[entrada]  = base fisica (16 bits altos) | tamano (16 bits bajos)
     registro (CS..) = numero de entrada (16 bits altos) | offset 0
   Devuelve 0 si el proceso entra en la memoria o -1 si no entra. */
int32_t iniciaMaquinaVirtual(MaquinaVirtual *vm, Cabecera cabecera){
    // registro que apunta a cada segmento, en el mismo orden que SEG_*
    static const int32_t regSegmento[CANT_SEG_PROCESO] = { REG_PS, REG_KS, REG_CS, REG_DS, REG_ES, REG_SS };
    uint32_t base, entrada;
    int32_t k, i;

    base = 0;
    entrada = 0;
    for (k = 0; k < CANT_SEG_PROCESO; k++){
        if (cabecera.tamanos[k] > 0){
            vm->segmentos[entrada] = (int32_t)((base << 16) | (cabecera.tamanos[k] & 0xFFFFu));
            vm->registros[regSegmento[k]] = (int32_t)(entrada << 16);
            base += cabecera.tamanos[k];
            entrada++;
        }
        else
            vm->registros[regSegmento[k]] = -1;     // segmento inexistente (0xFFFFFFFF)
    }
    for (i = (int32_t)entrada; i < CANT_SEGMENTOS; i++)    // entradas sin usar
        vm->segmentos[i] = -1;

    if (base > vm->memoria.tamano){
        printf("Error: memoria insuficiente\n");
        return -1;
    }

    // IP: entrada del Code Segment en los 16 bits altos (copiada de CS) y el entry point en los bajos
    vm->registros[REG_IP] = (int32_t)((uint32_t)vm->registros[REG_CS] | (cabecera.entryPoint & 0xFFFFu));

    // SP: la pila arranca vacia, apuntando justo despues del final del Stack Segment
    // (crece hacia abajo, asi que el primer PUSH le resta 4). En la version 1 no hay pila.
    if (vm->registros[REG_SS] != -1)
        vm->registros[REG_SP] = vm->registros[REG_SS] + (int32_t)cabecera.tamanos[SEG_STACK];
    else
        vm->registros[REG_SP] = 0;

    for (i = REG_EAX; i <= REG_EFX; i++)  //inicia los de proposito general
        vm->registros[i] = 0;
    return 0;
}
int32_t cantidadOperandosALeer(int32_t codigo) {
    if (codigo == 0x0F)
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
        else if (tipo==3){                  //Memoria
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
void limpiarBufferEntrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ; //descarta
}
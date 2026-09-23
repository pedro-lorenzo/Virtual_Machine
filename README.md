# Virtual Machine

Máquina virtual para el TP de Fundamentos de la Arquitectura de Computadoras (FI-UNMdP). Ejecuta programas en lenguaje máquina (`.vmx`) según la especificación de la cátedra, incluyendo desensamblador integrado.

## Requisitos

- `gcc` con soporte C11
- `make`

## Compilación

Parado en la raíz del proyecto:

```bash
make
```

Esto genera el ejecutable `vmx` en la raíz. `make clean` lo elimina.

## Uso

`vmx` ejecuta un programa ya traducido a lenguaje máquina (`.vmx`). Para generar ese archivo a partir de código fuente Assembler (`.asm`) se usa el traductor `vmt`, provisto por la cátedra en `herramientas/`.

```bash
# 1. Traducir el .asm a .vmx
./herramientas/vmt programa.asm programa.vmx

# 2. Ejecutar el programa
./vmx programa.vmx

# (opcional) mostrar el desensamblado antes de ejecutar
./vmx programa.vmx -d
```

## Estructura del proyecto

```
Headers/       Archivos .h (tipos, registros, instrucciones, etc.)
src/           Implementación de la máquina virtual
herramientas/  Traductor vmt provisto por la cátedra (Linux y Windows)
docs/          Especificación de la máquina virtual y del lenguaje Assembler
test/          Programas .asm/.vmx de prueba
Makefile       Compila src/*.c en el ejecutable vmx
```


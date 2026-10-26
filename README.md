# Virtual Machine

Máquina virtual para el TP de Fundamentos de la Arquitectura de Computadoras (FI-UNMdP). Ejecuta programas en lenguaje máquina (`.vmx`, versión 1 o 2) según la especificación de la cátedra, incluyendo desensamblador integrado y soporte de imágenes de estado (`.vmi`).

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
# 1. Traducir el .asm a .vmx (v=1 o v=2 según la versión del programa)
./herramientas/vmt programa.asm programa.vmx v=2

# 2. Ejecutar el programa
./vmx programa.vmx [m=M] [programa.vmi] [-d] [-p param1 param2 ...]
```

| Argumento | Descripción |
|---|---|
| `programa.vmx` | Programa a ejecutar (versión 1 o 2). |
| `m=M` | Tamaño de la memoria en KiB (por defecto 16). |
| `programa.vmi` | Archivo de imagen: se guarda en cada breakpoint. Sin `.vmx`, se reanuda desde esa imagen. |
| `-d` | Muestra el desensamblado antes de ejecutar. |
| `-p ...` | Parámetros para el programa (siempre al final). |

```bash
./vmx programa.vmx -d
./vmx programa.vmx m=32 imagen.vmi
./vmx imagen.vmi
```

## Estructura del proyecto

```
Headers/       Archivos .h (tipos, registros, instrucciones, etc.)
src/           Implementación de la máquina virtual
herramientas/  Traductor vmt y depurador vmg provistos por la cátedra
docs/          Especificación de la máquina virtual y del lenguaje Assembler
test/          Programas .asm de prueba
Makefile       Compila src/*.c en el ejecutable vmx
```

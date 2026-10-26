\\STACK 64
SALUDO EQU "Hola\n"
main: MOV EDX, KS
      ADD EDX, SALUDO
      SYS 4
      SYS 0
      SYS 4
      STOP
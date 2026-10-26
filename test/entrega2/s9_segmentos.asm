; Copia un string del Const Segment (KS) al Extra Segment (ES) byte a byte y lo imprime desde ES
; Ejecutar: vmx s9_segmentos.vmx
; Esperado: Hola Arq
; Con -d: se ve la constante en el Const Segment y el marcador > en la entrada
\\DATA 16
\\EXTRA 32
\\STACK 64
SALUDO EQU "Hola Arq\n"
main:  mov ebx, ks
       add ebx, SALUDO
       mov eex, es
copia: mov efx, b[ebx]
       mov b[eex], efx
       cmp efx, 0
       jz listo
       add ebx, 1
       add eex, 1
       jmp copia
listo: mov edx, es
       sys 4
       stop

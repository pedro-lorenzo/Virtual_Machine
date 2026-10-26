; SYS 3 (leer string, maximo 30), SYS 4 (escribir string) y longitud del string
; Ejecutar: vmx s4_strings.vmx
; Entrada -> Salida esperada:
;   Arquitectura -> "Arquitectura" y longitud 12
;   (linea vacia) -> linea vacia y longitud 0
\\DATA 64
\\STACK 64
NL EQU "\n"
main:  mov edx, ds
       mov ecx, 30           ; maximo 30 caracteres
       sys 3                 ; leer string
       mov edx, ds
       sys 4                 ; mostrarlo
       mov edx, ks
       add edx, NL
       sys 4
       mov ebx, ds
       mov eax, 0
lazo:  mov ecx, b[ebx]
       cmp ecx, 0
       jz fin
       add eax, 1
       add ebx, 1
       jmp lazo
fin:   mov [ds+40], eax
       mov edx, ds
       add edx, 40
       mov eax, 1
       ldl ecx, 1
       ldh ecx, 4
       sys 2                 ; longitud
       stop

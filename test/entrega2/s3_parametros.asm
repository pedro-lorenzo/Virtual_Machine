; Param Segment: imprime argc y cada argv
; Ejecutar: vmx s3_parametros.vmx -p hola mundo
; Esperado: argc = 3, luego "s3_parametros.vmx", "hola" y "mundo" (una por linea)
; Sin -p: argc = 0 y no imprime strings
\\DATA 16
\\STACK 64
NL EQU "\n"
main:  mov eex, [sp+4]       ; argc
       mov [ds+0], eex
       mov edx, ds
       ldl ecx, 1
       ldh ecx, 4
       mov eax, 1
       sys 2
       mov eax, [sp+4]
       mov ebx, [sp+8]       ; argv
sig:   cmp eax, 0
       jz fin
       mov edx, [ebx]        ; puntero al string
       sys 4
       mov edx, ks
       add edx, NL
       sys 4
       add ebx, 4
       sub eax, 1
       jmp sig
fin:   ret

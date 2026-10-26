; Factorial recursivo con PUSH/CALL/RET (n por teclado, decimal)
; Traducir: vmt s1_factorial.asm s1_factorial.vmx v=2
; Ejecutar: vmx s1_factorial.vmx
; Entrada -> Salida esperada:
;   5 -> 120 | 0 -> 1 | 1 -> 1 | 10 -> 3628800 | 12 -> 479001600
\\DATA 16
\\STACK 256
main:  mov eax, 1            ; leer en decimal
       mov edx, ds
       ldl ecx, 1
       ldh ecx, 4
       sys 1                 ; n queda en [DS]
       mov ebx, [ds+0]
       push ebx              ; argumento
       call fact
       pop ebx               ; limpiar argumento
       mov [ds+0], eax
       mov eax, 1
       mov edx, ds
       ldl ecx, 1
       ldh ecx, 4
       sys 2                 ; escribir n!
       stop

fact:  push ebx
       mov ebx, [sp+8]       ; n
       cmp ebx, 1
       jp rec
       mov eax, 1
       jmp fin
rec:   sub ebx, 1
       push ebx
       call fact             ; eax = (n-1)!
       pop ebx
       mov ebx, [sp+8]
       mul eax, ebx
fin:   pop ebx
       ret

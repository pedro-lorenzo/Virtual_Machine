; Fibonacci recursivo (doble llamada), n por teclado
; Ejecutar: vmx s5_fibonacci.vmx
; Entrada -> Salida esperada:
;   0 -> 0 | 1 -> 1 | 2 -> 1 | 7 -> 13 | 10 -> 55 | 15 -> 610
\\DATA 16
\\STACK 1024
main:  mov eax, 1
       mov edx, ds
       ldl ecx, 1
       ldh ecx, 4
       sys 1
       mov ebx, [ds+0]
       push ebx
       call fib
       pop ebx
       mov [ds+0], eax
       mov eax, 1
       mov edx, ds
       ldl ecx, 1
       ldh ecx, 4
       sys 2
       stop

fib:   push ebx
       push ecx
       mov ebx, [sp+12]      ; n
       cmp ebx, 2
       jn base
       sub ebx, 1
       push ebx
       call fib              ; eax = fib(n-1)
       pop ebx
       mov ecx, eax
       sub ebx, 1
       push ebx
       call fib              ; eax = fib(n-2)
       pop ebx
       add eax, ecx
       jmp fin
base:  mov eax, ebx
fin:   pop ecx
       pop ebx
       ret

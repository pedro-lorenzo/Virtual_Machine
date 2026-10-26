; Suma de 4 enteros leidos por teclado, con subrutina que recibe puntero y cantidad por pila
; Ejecutar: vmx s2_suma_vector.vmx
; Entrada -> Salida esperada:
;   1 2 3 4 (uno por linea) -> 10
;   10 -3 7 0 -> 14
\\DATA 16
\\STACK 128
main:  mov eax, 1
       mov edx, ds
       ldl ecx, 4            ; 4 valores
       ldh ecx, 4            ; de 4 bytes
       sys 1
       push 4                ; cantidad
       mov edx, ds
       push edx              ; puntero al vector
       call suma
       pop ecx
       pop ecx
       mov [ds+0], eax
       mov eax, 1
       mov edx, ds
       ldl ecx, 1
       ldh ecx, 4
       sys 2
       stop

suma:  push ebx
       push ecx
       push edx
       mov edx, [sp+16]      ; puntero
       mov ecx, [sp+20]      ; cantidad
       mov eax, 0
lazo:  cmp ecx, 0
       jz fin
       mov ebx, [edx]
       add eax, ebx
       add edx, 4
       sub ecx, 1
       jmp lazo
fin:   pop edx
       pop ecx
       pop ebx
       ret

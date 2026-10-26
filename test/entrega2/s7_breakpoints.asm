; Suma 1..5 con un breakpoint (SYS 0xF) por vuelta
; Ejecutar: vmx s7_breakpoints.vmx imagen.vmi     (y mirar con: vmg imagen.vmi -r -s)
; En cada breakpoint: g = sigue | Enter = un paso | q = termina
; Estado en el breakpoint n (EAX / ECX): 1: 1/1 | 2: 3/2 | 3: 6/3 | 4: 10/4 | 5: 15/5
; Al terminar imprime 15
; Sin imagen.vmi los breakpoints se ignoran y imprime 15 directo
\\DATA 16
\\STACK 64
main:  mov eax, 0
       mov ecx, 1
lazo:  add eax, ecx
       sys 0xF               ; breakpoint en cada vuelta
       add ecx, 1
       cmp ecx, 6
       sys 0
       jn lazo
       mov [ds+0], eax
       mov eax, 1
       mov edx, ds
       ldl ecx, 1
       ldh ecx, 4
       sys 2
       stop

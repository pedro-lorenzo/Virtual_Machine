; Recursion infinita
; Ejecutar: vmx s8a_stack_overflow.vmx
; Esperado: Error: stack overflow
\\STACK 64
main:  call main

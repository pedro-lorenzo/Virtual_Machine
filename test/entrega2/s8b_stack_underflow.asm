; 4 POP con solo 3 palabras iniciales en la pila (retorno, argc, argv)
; Ejecutar: vmx s8b_stack_underflow.vmx
; Esperado: Error: stack underflow
\\STACK 64
main:  pop eax
       pop eax
       pop eax
       pop eax
       stop

; Data de 20000 bytes no entra en los 16 KiB por defecto
; Ejecutar: vmx s8d_memoria_insuficiente.vmx
; Esperado: Error: memoria insuficiente
; Ejecutar: vmx s8d_memoria_insuficiente.vmx m=64
; Esperado: termina sin error
\\DATA 20000
\\STACK 64
main:  stop

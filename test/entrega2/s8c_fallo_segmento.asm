; Lectura en DS+16 con Data de 16 bytes
; Ejecutar: vmx s8c_fallo_segmento.vmx
; Esperado: Fallo de segmento
\\DATA 16
\\STACK 64
main:  mov eax, [ds+16]
       stop

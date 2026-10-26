; Sectores de registro AL, AH, AX (se imprime EAX en hexadecimal)
; Ejecutar: vmx s6_sectores.vmx
; Salida esperada (4 lineas):
;   0x3412 | 0xffffff00 | 0xffff00ff | 0xffff1234
\\DATA 16
\\STACK 64
NL EQU "\n"
main:  mov eax, 0
       mov al, 0x12
       mov ah, 0x34
       mov ebx, eax
       call imp              ; 0x00003412
       mov eax, -1
       mov al, 0
       mov ebx, eax
       call imp              ; 0xFFFFFF00
       mov eax, -1
       mov ah, 0
       mov ebx, eax
       call imp              ; 0xFFFF00FF
       mov eax, -1
       mov ax, 0x1234
       mov ebx, eax
       call imp              ; 0xFFFF1234
       stop

imp:   push eax
       push ecx
       push edx
       mov [ds+0], ebx
       mov edx, ds
       ldl ecx, 1
       ldh ecx, 4
       mov eax, 0x08
       sys 2
       pop edx
       pop ecx
       pop eax
       ret

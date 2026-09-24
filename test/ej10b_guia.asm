		
		MOV EAX, 1
		MOV EDX, DS
		LDL ECX, 1
		LDH ECX, 4
		MOV EFX, 0		; EFX, acumulo la primer suma
		MOV EBX, 0
OTRO_1:		SYS 1
		CMP [EDX], 0
		JN FIN_INGRESO
		ADD EBX, 1
		ADD EFX, [EDX]
		JMP OTRO_1
FIN_INGRESO:	MOV EEX, 0		; EEX acumula 2da suma
		MOV AC, 1		; AC cuenta los ingresados, arranca en uno entonces al ser igual que EBX corta (ECX no, porque lo usa SYS)
OTRO_2:		CMP AC, EBX
		JZ FIN_2
		SYS 1
		ADD EEX, [EDX]
		ADD AC, 1
		JMP OTRO_2
FIN_2:		SUB EFX, EEX
		MOV [DS], EFX		; guardo el resultado donde lo lee SYS 2
		MOV EAX, 1
		MOV EDX, DS
		LDL ECX, 1
		LDH ECX, 4
		SYS 2
		STOP
		
		
		

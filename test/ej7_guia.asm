		MOV EAX, 1
		LDL ECX, 1
		LDH ECX, 4
		MOV EDX, DS
		SYS 1
		MOV EBX, [DS];	Almacena el valor ingresado
		CMP EBX, 1
		JZ NO_PRIMO
		CMP EBX, 2
		JZ PRIMO
		MOV EFX, 2;	Se utiliza para el divisor	
OTRO:		DIV EBX, EFX
		CMP AC, 0
		JZ NO_PRIMO
		ADD EFX, 1
		MOV EBX, [DS]
		CMP EFX, EBX
		JZ PRIMO
		JMP OTRO
NO_PRIMO: 	MOV [DS+4], 0
		JMP FIN
PRIMO:		MOV [DS+4], 1
FIN:		MOV EDX, DS
		ADD EDX, 4
		MOV EAX, 1
		LDL ECX, 1
		LDH ECX, 4
		SYS 2
		STOP


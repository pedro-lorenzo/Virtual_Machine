		MOV EAX, 1
		LDL ECX, 1
		LDH ECX, 4
		MOV EDX, DS
		SYS 1
		MOV EFX, [DS]
		MOV EEX, 0; contador, me va a servir para ir cargando los bits en memoria
OTRO:	ADD EEX, 4
		DIV EFX, 2
		MOV EBX, DS
		ADD EBX, EEX
		MOV [EBX], AC
		CMP EFX, 0
		JP OTRO
		JN OTRO
		MOV EAX, 1
		LDL ECX, 1
		LDH ECX, 4
MUESTRA:	MOV EDX, DS
		ADD EDX, EEX
		SYS 2
		SUB EEX, 4
		CMP EEX, 0
		JP MUESTRA
		STOP
	
	
	
	


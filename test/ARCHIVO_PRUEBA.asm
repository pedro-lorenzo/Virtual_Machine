	MOV EAX, 1
	MOV EDX, DS
	LDL ECX, 1
	LDH ECX, 4
	MOV EEX, 0
	MOV EFX, 0
OTRO:	SYS 1
	CMP [DS],-1
	JNP FIN
	ADD EFX, [DS]; suma el valor que esta en la celda apuntada por DS
	ADD EEX, 1
	JMP OTRO
FIN:	CMP EEX, 1
	JN DETENER_EJECUCION
	DIV EFX, EEX
	MOV [DS], EFX; guardo el promedio en memoria
	MOV EDX, DS; hago que edx apunte a la celda donde esta el resultado
	LDL ECX, 1
	LDH ECX, 4
	MOV EAX, 1
	SYS 2
DETENER_EJECUCION:	STOP

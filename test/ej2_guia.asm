			MOV EDX, DS
			MOV EAX, 1
			LDL ECX, 1
			LDH ECX, 4
			MOV EFX, 0 ; registro sobre el que se calcula el numero en decimal
			MOV EEX, 1; registro que va acumulando la potencia
OTRO:		SYS 1
			CMP [DS], 0
			JN FIN
			JZ ES_CERO	
			CMP [DS], 1
			JP FIN
			ADD EFX, EEX; si llega aca se ingreso un 1
ES_CERO:	MUL EEX, 2; equivalente: SHL EEX, 1
			JMP OTRO
FIN:		MOV [DS], EFX
			MOV EDX, DS
			LDL ECX, 1
			LDH ECX, 4
			MOV EAX, 1
			SYS 2
		MOV EAX, 1
		MOV EDX, DS
		LDL ECX, 1
		LDH ECX, 4
		MOV EFX, 0 		;efx lo utilizo de N (dimension logica del vector)
CARGA_VEC:	SYS 1
		CMP [EDX], 0
		JN FIN_INGRESO		;si es negativo termino de ingresar
		ADD EFX, 1 		;N += 1
		ADD EDX, 4 		;(v+N)
		JMP CARGA_VEC	
FIN_INGRESO:	MOV EBX, 1
DESCARGA_VEC:	CMP EBX,EFX
		JZ FIN_DESCARGA
		SYS 1
		MOV EEX, DS 		;EEX, lo utilizo de i, para recorrer el vector hasta encontrar el elemento leido
		ADD EBX, 1
BUSCA:		CMP [EEX], [EDX] 	;EEX Apunta a los distintos elems del vector, edx al ultimo leido 
		JZ PISA
		ADD EEX, 4
		CMP EEX,EDX
		JZ ERROR
		JMP BUSCA
PISA:		MOV [EEX],0		;Encontre el elemento ingresado, lo piso y pido otro
		JMP DESCARGA_VEC
ERROR:		STOP
FIN_DESCARGA:	MOV EEX, DS		;Me quedo todo el vector con ceros, excepto el valor no ingresado, lo busco y lo muestro
WHILE:		CMP [EEX], 0
		JP ENCONTRE
		JN ENCONTRE
		ADD EEX, 4
		JMP WHILE
ENCONTRE:	MOV EDX, EEX
		MOV EAX, 1
		LDL ECX, 1
		LDH ECX, 4
		SYS 2
		STOP
		

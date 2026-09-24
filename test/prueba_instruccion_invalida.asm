; Prueba de "instruccion invalida".
; El ensamblador no genera codigos de operacion inexistentes, asi que el
; programa se modifica a si mismo: escribe 0x0B (hueco de la tabla, 0x0B-0x0E)
; sobre la instruccion siguiente y despues la ejecuta.
		LDH EAX, 0x0B00		; byte mas alto de EAX = 0x0B
		LDL EAX, 0		; EAX = 0x0B000000
		MOV [CS+13], EAX	; 13 = offset de AQUI (4 + 4 + 5 bytes)
AQUI:		STOP			; se pisa con 0x0B 00 00 00
		STOP			; relleno: el MOV escribe 4 bytes

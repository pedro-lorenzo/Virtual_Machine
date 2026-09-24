; Prueba de "fallo de segmento".
; Lee 4 bytes a partir de un offset que queda fuera del segmento de datos
; (el segmento de datos ocupa lo que queda de los 16 KiB despues del codigo).
		MOV EDX, DS
		MOV EAX, [EDX+16380]	; 16380 + 4 > tamano del segmento de datos
		MOV EAX, 99		; no deberia llegar a ejecutarse
		STOP

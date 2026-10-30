\\EXTRA 1024
            ; CALL heap_init
            ; sin parámetros
            ; asume que todo el extra segment es para uso de memoria dinámica    

heap_init:  MOV [ES], ES
            ADD [ES], 4
            RET

NULL EQU -1
SIZE_EXTRA EQU 1024
            ; PUSH <<CANT_BYTES>>
            ; CALL alloc
            ; ADD SP, 4
            ; devuelve en EAX dirección de memoria o NULL si no hay memoria suficiente

alloc:      PUSH    BP
            MOV     BP, SP
            PUSH    BX              ; 2 bytes menos sign. de BX
            MOV     EAX, NULL
            MOV     BX, [ES]
            ADD     BX, [BP+8]      ; BX += CANT_BYTES
            CMP     BX, SIZE_EXTRA
            JP      alloc_end
            MOV     EAX, [ES]
            ADD     [ES], [BP+8]
alloc_end:  POP     BX
            MOV     SP, BP  
            POP     BP
            RET


    .386P               ; Priviledged instructions can be used.



_TEXT	SEGMENT PARA USE32 PUBLIC 'CODE'
_TEXT	ENDS
_DATA	SEGMENT DWORD USE32 PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT DWORD USE32 PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT DWORD USE32 PUBLIC 'BSS'
_BSS	ENDS
_TLS	SEGMENT DWORD USE32 PUBLIC 'TLS'
_TLS	ENDS

; Put all segment directives into one single group (flat memory model).

;GFLAT	GROUP _TEXT, _DATA, CONST, _BSS
;		ASSUME	CS: GFLAT, DS: GFLAT, ES: GFLAT, SS: GFLAT


_TEXT	SEGMENT
ASSUME	CS:_TEXT, DS:_TEXT, ES:_TEXT, SS:_TEXT

_simpleISR PROC NEAR
    iretd		        ; do nothing
_simpleISR ENDP


EXTRN _pCurrentThreadParameters:DWORD
EXTRN _pNextThreadParameters:DWORD

_contextSwitchISR PROC NEAR
    pushad
    mov eax, _pCurrentThreadParameters
    mov dword ptr [eax], esp
    
    mov eax, _pNextThreadParameters
    mov esp, dword ptr [eax]
    
    popad
    iretd
_contextSwitchISR ENDP


EXTRN _handleTick:NEAR
savedStack DD 0h

_tickISR PROC NEAR
    pushad              ; save the current context
        
    mov al,20h          ; Send an end-of-interrupt to the intel i8259 programmable interrupt controllers
    out 20h,al

    mov savedStack, esp ; save the current stack for later
    call _handleTick
    
    popad
    iretd
_tickISR ENDP



_executeContextSwitchFromISR PROC NEAR
    mov esp, savedStack ; restore the stack to what it was when the interrupt had just occurred
    
    mov eax, _pCurrentThreadParameters
    mov dword ptr [eax], esp
    
    mov eax, _pNextThreadParameters
    mov esp, dword ptr [eax]
    
    popad
    iretd
_executeContextSwitchFromISR ENDP


_TEXT ENDS
		
END

; MJVMK Boot Sector (C) Copyright Sean Foley 2002
; assemble with:
; nasm -f bin -o bootsect.bin bootsect.asm

    bits 16
    org 0

start:      
    
    jmp short begin
    nop

           
bsOEM       db "MJVMK   "               ; OEM String
bsSectSize  dw 512                      ; Bytes per sector
bsClustSize db 1                        ; Sectors per cluster
bsResSects  dw 1                        ; # of reserved sectors
bsFatCnt    db 2                        ; # of fat copies
bsRootSize  dw 224                      ; size of root directory
bsTotalSect dw 2880                     ; total # of sectors if < 32 meg
bsMedia     db 0xF0                     ; Media Descriptor
bsFatSize   dw 9                        ; Size of each FAT
bsTrackSect dw 18                       ; Sectors per track
bsHeadCnt   dw 2                        ; number of read-write heads

bsHidenSect dw 0                        ; number of hidden sectors
bsCurTrack  dw 0                        ; starts out as 2nd word of number of hidden sectors

bsHugeSect  dd 0                        ; 
bsBootDrive dw 0                        ; holds the drive we're booting from

bsBootSign  db 29h                      ; boot signature 29h
bsSecRead   dw 0                        ; starts out as first word of volume ID
bsHead      dw 0                        ; starts out as second word of volumeID
bsVoLabel   db "NO NAME    "            ; Volume Label
bsFSType    db "FAT12   "               ; File System type



begin:

    cld                                 ; clear direction flag so string operations increment index registers
    cli                                 ; disable interrupts
    mov ax,0x9d80                       ; set the stack pointer, can't go much higher than this
                                        ; free memory extends from 0x7e00 (end of boot sector) to a0000 (0x98200 bytes)       
    mov ss,ax
    mov sp, 0x0280                      ; this give is just enough stack space, and any more puts us past the a0000 memory limit
    sti                                 ; enable interrupts

    ; BIOS puts us at 0:0x7C00, so we set ds accordingly below

    mov es,ax                           ; we will copy this boot sector to a new destination, the segment in ax
    xor di,di                           ; dest offset is 0
    mov bx,0x7c0
    mov ds,bx                           ; source segment is 0x7c0
    xor si,si                           ; source ofset is 0
    mov cx,0x100                        ; copy 256 words (512 bytes)
    rep movsw                           ; do the copy

    
    jmp 0x9d80:go                       ; jump to the next instruction in our new location


go:
    mov ds,ax                           ; set the data segment to our new location

    ; scroll up display window
    mov ax,0x600
    mov bh,0x7
    xor cx,cx
    mov dx,0x1850
    int 0x10                            ; scroll to window location - ch, cl contain upper left corner
                                        ; dh, dl contains lower right corner (24, 80)
                                        ; INT 10 - VIDEO - SCROLL UP WINDOW
    ; set cursor position
    mov ah,0x2
    xor bh,bh
    xor dx,dx
    int 0x10                            ; set cursor position - dh,dl is row and column - bh is page number
                                        ; INT 10 - VIDEO - SET CURSOR POSITION
    ; make cursor invisible
    mov ah, 0x01
    mov cx, 0x0100
    int 0x10                            ; INT 10 - VIDEO - SET TEXT-MODE CURSOR SHAPE

    lea si,[bsStartLine]
    call putStr                         ; print the title banner
    call resetDrive
    
    xor ah,ah
    mov al,[bsFatCnt]    
    
    mul word [bsFatSize]                ; dx:ax constains result of multiply    
    add ax,[bsHidenSect]                ; add the full 4 byte quantity at bsHidenSect to dx:ax
    adc dx,[bsCurTrack]                 ; this is the second word
    add ax,[bsResSects]
    mov cx,ax                           ; cx containts total fat, huge, hidden, and reserved sectors
    mov ax,0x20
    mul word [bsRootSize]
    div word [bsSectSize]               ; ax contains sectors in root directory
    add ax,cx                           ; ax contains total root, fat, huge, hidden, and reserved sectors

    lea si,[bsLoadLine]
    call putStr                         ; print 'loading' 



    div word [bsTrackSect]              ; divide dx:ax by sectors per track, ax contains quotient, dx contains remainder
    mov	[bsSecRead],dl
    xor dx,dx
    div word [bsHeadCnt]
    mov [bsHead], dl
	

    call readDisk


doneReading:

    mov al,0xc                          ; al contains 12
    mov dx,0x3f2                        ; dx containds 1010
    out dx,al                           ; stops floppy motor
    cli                                 ; disable the interrupts

    ; we now copy the code to a new memory location
    mov ax, 0x2000                      ; starting system segment goes here	                            	    
    mov ds,ax                           ; ds points to the linear address 128k
    xor ax,ax
    mov es,ax                           ; es points to 0
    ; mov bx,0x7                          ; we will do 7 64KB copies  (***) SIZEINC
    mov bx,0x6                          ; we will do 6 64KB copies  (***)
    ; mov bx,0x10                          ; we will do 16 64KB copies  (***)
    ;mov bx,[bsSysSize]                  ; the number of copies we will do
    mov di, 0x1000                      ; the first copy will skip the first 0x1000 bytes, the lower memory area
    mov si, 0x1000
    mov cx, 0x7800

doKernelCopy:

    rep movsw                           ; repeat word copy from ds:si to es:di

    dec bx
    jz finishedCopy                     ; if no more data to copy, jump
    mov ax,ds
    add ax,0x1000
    mov ds,ax                           ; increment ds by 64k
    mov ax,es
    add ax,0x1000
    mov es,ax                           ; increment es by 64k

    xor si,si
    xor di,di
    mov cx,0x8000                       ; we will repeat the move 32K times

    jmp short doKernelCopy              ; more data to copy

finishedCopy:

    xor ax,ax
    mov ds,ax
    jmp 0x0:0x1000                      ; jump to the entry point at offset 1000h  START THE O/S





; This routine loads the system at the given segment, making sure
; no 64kB boundaries are crossed. We try to load it as fast as
; possible, loading whole tracks whenever we can.
;
; in:	bsSecRead, bsHead
;


readDisk:

    mov bx,0x2000                       ; 0x2000 is the starting system segment, must be at a 64 kb boundary
    mov es,bx 
	xor bx,bx		                    ; bx is starting address within segment

repeatRead:

	mov ax,es
    sub ax,0x2000                       ; the starting system segment is here as well
    
    ;xor dx,dx
    ;mov cx, 0x1000
    ;div cx ; dx:ax is divided by cx, result is in ax
    ;inc ax
    ;cmp ax, [bsSysSize]
    
    ; cmp ax, 0x7000                      ; the system size is 7 64 kb segments SIZEINC for some reason, cannot go larger than 6 64 kb segments
    cmp ax, 0x6000
    jbe readMoreData
	ret

readMoreData:

	mov ax,[bsTrackSect]
	sub ax,[bsSecRead]
	mov cx,ax
	shl cx,9
	add cx,bx
	jnc readSectors
	je readSectors
	xor ax,ax
	sub ax,bx
	shr ax,9

readSectors:                            ; ax contains number of sectors to read

	call readTrack
	mov cx,ax
	add ax,[bsSecRead]      
	cmp ax,[bsTrackSect]
	jne setSectorsAlreadyRead
	mov ax,1
	sub ax,[bsHead]
	jne setHead
	inc word [bsCurTrack]

setHead:

	mov [bsHead],ax
	xor ax,ax

setSectorsAlreadyRead:

	mov [bsSecRead],ax
	shl cx,9
	add bx,cx
	jnc repeatRead
	mov ax,es
	add ah,0x10
    mov es,ax
	xor bx,bx
	jmp short repeatRead

readTrack:

	pusha
    lea si,[bsDot]
    call putStr                         ; print a dot
	mov	dx,[bsCurTrack]
	mov	cx,[bsSecRead]
	inc	cx
	mov	ch,dl
	mov	dx,[bsHead]
	mov	dh,dl
	and	dx, 0x0100
	mov	ah, 2
	int	0x13                            ; INT 13 - DISK - READ SECTOR(S) INTO MEMORY
	jc	badRead
	popa
	ret

badRead:
	
    call resetDrive	
	popa	
	jmp short readTrack


resetDrive:                             ; note that this function clobbers ah and dl
    mov dl,[bsBootDrive]
    xor ah,ah
    int 0x13                            ; INT 13 - DISK - RESET DISK SYSTEM
    jc near bootFail
    ret


bootFail:

    lea si,[bsDiskError]
    call putStr
    jmp short $                         ; infinite loop


putStr:

    pusha
    mov ah,0xe                          ; we put a character
    mov bx,0x7                          ; attribute

getByte:

    lodsb                               ; load string byte at ds:si into al
    or al,al
    jnz putByte                         ; check for end of string
    popa
    ret

putByte:

    int 0x10                            ; output to screen INT 10 - VIDEO - TELETYPE OUTPUT
    jmp short getByte                   ; load next character


;bsSysSize dw 6                          ; number of 64kb segments to be loaded
bsDot db '.',0
bsStartLine db 'MJK, Copyright (C) 2002, MJK corp.',10,13,0
bsLoadLine db 'Loading',0
bsDiskError db 'Disk error',0
times 510-($-$$) db 0
BootSig     db 0x55, 0xAA





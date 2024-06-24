org 0x0000

; Set video mode
mov ah, 0x00 ; Set video mode
mov al, 0x03 ; 80x25 text mode
int 0x10

; Set color
mov ah, 0x0B
mov bh, 0x00
mov bl, 0x01
int 0x10


mov si, msg
call print_string


; Get input
cmd_input:
    mov si, cmd_prompt
    call print_string
    mov di, cmds
    call get_input
    jmp $

; Get input function
get_input:
    xor cx, cx  ; Clear CX to use as character counter

.input_loop:
    mov ah, 0x00 ; Read from keyboard
    int 0x16 ; Keyboard interrupt

    cmp al, 0x08  ; Check if backspace
    je .handle_backspace

    cmp al, 0x0D  ; Check if enter
    je .process_input

    

    mov ah, 0x0E ; Print character
    int 0x10 ; Print character
    stosb  ; Store character and increment DI
    inc cx  ; Increment character count
    jmp .input_loop

.handle_backspace:
    test cx, cx  ; Check if we're at the start of the input
    jz .input_loop  ; If so, ignore backspace

    dec di  ; Move back one character in the buffer
    dec cx  ; Decrement character count

    mov ah, 0x0E
    mov al, 0x08  ; Backspace
    int 0x10
    mov al, ' '   ; Space (to erase the character)
    int 0x10
    mov al, 0x08  ; Backspace again (to move cursor back)
    int 0x10

    jmp .input_loop

.process_input:
    mov byte [di], 0 ; Null-terminate the input string
    mov ah, 0x0E  ; Print character
    mov al, 0x0D  ; Carriage return
    int 0x10      
    mov al, 0x0A  ; Line feed
    int 0x10
    jmp process_commands
process_commands: 
    mov byte [di], 0 ; Move null terminator to end of input
    mov si, cmds ; Point to start of input
    call execute_command ; Execute command
    jmp cmd_input ; Loop

execute_command:
    ; Compare with "home"
    mov di, cmd_return_to_home
    call strcmp
    jc return_to_kernel

    ; Compare with "ls"
    mov di,show_files_cmd
    call strcmp
    jc show_files

    ; If no match, it's an error
    jmp error

; String comparison function
; SI = input string, DI = command to compare
; Carry flag set if strings match
strcmp:
    push si
    push di
.loop:
    mov al, [si]
    mov bl, [di]
    cmp al, bl
    jne .not_equal
    cmp al, 0
    je .equal
    inc si
    inc di
    jmp .loop
.not_equal:
    clc  ; Clear carry flag (no match)
    jmp .done
.equal:
    stc  ; Set carry flag (match)
.done:
    pop di
    pop si
    ret

; Error function
error:
    mov si, invalid_cmd_msg
    call print_string
    jmp cmd_input

show_files:
    mov si, files
    call print_string
    jmp cmd_input


return_to_kernel:
    mov ax, 0x2000
    mov ds, ax
    mov es, ax

    call 0x2000:0x0000

print_string:
    mov ah, 0x0E
    mov bh, 0x0
    mov bl, 0x1

print_char:
    lodsb
    cmp al, 0
    je print_done
    int 0x10
    jmp print_char

print_done:
    ret

msg db '-------------------------------------------', 0xA, 0xD,0x20,0x20,0x20,0x20,'File Table ', 0xA, 0xD, \
'-------------------------------------------', 0xA, 0xD,0
invalid_cmd_msg db 10,13,'Invalid Command',10,13,0
cmds: times 64 db 0
cmd_return_to_home db 'ret', 0
cmd_prompt db 10,13,'|>:', 0
show_files_cmd db 'ls', 0
files db 10,13,'main.asm',0x20,'hello.txt', 10,13,0

times 512-($-$$) db 0

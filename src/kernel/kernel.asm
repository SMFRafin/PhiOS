org 0x0000 

; Set video mode
mov ah,0x00
mov al,0x03 ; 80x25 text mode
int 0x10

; Set color
mov ah, 0x0B
mov bh, 0x00
mov bl, 0x01
int 0x10

; Print kernel messages
mov si, kernel_msg1
call print_string

jmp cmd_input

;Error function 
error:
    mov si, invalid_cmd_msg
    call print_string
    jmp cmd_input

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
    mov si, cmds ; Point to start of input
    call execute_command ; Execute command
    jmp cmd_input ; Loop

execute_command:
    ; Compare with "reboot"
    mov di, reboot_cmd
    call strcmp
    jc reboot

    ; Compare with "dirs"
    mov di, dirs_cmd
    call strcmp
    jc filetable

    ; Compare with "cls"
    mov di, cls_cmd
    call strcmp
    jc clear_screen
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

reboot:
    jmp 0xFFFF:0x0000 ; Reset vector

clear_screen:
    

filetable:
    ; Load the filetable segment
    mov ax, 0x1000
    mov ds, ax
    mov es, ax

    ; Call the filetable code
    call 0x1000:0x0000
    jmp cmd_input



print_string:
    mov ah, 0x0E ; Print character
    mov bh, 0x0 ; Set page
    mov bl, 0x1 ; Set color

print_char:
    lodsb ; Load byte from string
    cmp al, 0 ; Check if end of string
    je print_done ; If equal, jump to print_done
    int 0x10 ; Print character
    jmp print_char ; Loop

print_done:
    ret


kernel_msg1 db ' _         _   __',10,13
kernel_msg2 db '|_) |_  o / \ (_  ',10,13
kernel_msg3 db '|   | | | \_/ __) ',10,13,0

cmd_prompt db 10,13,'|>:', 0
reboot_cmd db 'reboot', 0
cmd_cmd db 'cmd', 0
dirs_cmd db 'dirs', 0
cls_cmd db 'cls', 0
invalid_cmd_msg db 10,13,'Invalid Command',10,13,0

cmds: times 64 db 0 ; Make space for 64 characters of input

times 1024-($-$$) db 0
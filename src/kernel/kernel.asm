org 0x0000 

; Set video mode
mov ah,0x00
mov al,0x03 ; 80x25 text mode
int 0x10

; Set color
mov ah, 0x0B
mov bh, 0x00
mov bl, 0x09
int 0x10

; Print kernel messages
start:
    mov si, kernel_msg1
    call print_string
    mov si, help_msg
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

    ; Compare with "help"
    mov di, help_cmd
    call strcmp
    jc help

    ;Compare with phi -v
    mov di, version_cmd
    call strcmp
    jc version

    ;Compare with phi -phi
    mov di,phi_phi_cmd
    call strcmp
    jc phi_phi_cmd_dis
    ; If no match, it's an error
    jmp error

; String comparison function
; SI = input string, DI = command to compare
; Carry flag set if strings match
strcmp:
    push si 
    push di
.loop:
    mov al, [si] ;get the input character
    mov bl, [di] ;get the command character
    cmp al, bl   ;compare the characters
    jne .not_equal ;if they don't match, jump to .not_equal
    cmp al, 0    ;check if we've reached the end of the input string
    je .equal    ; if so, jump to .equal
    inc si       ; increment both pointers
    inc di
    jmp .loop    ; keep iterating   
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

; Clear screen    
clear_screen:
; Set video mode
    mov ax, 0x0003 ; Clear screen
    int 0x10
    ;Reset video mode
    mov ah,0x00
    mov al,0x03 ; 80x25 text mode
    int 0x10

    ; Set color
    mov ah, 0x0B
    mov bh, 0x00
    mov bl, 0x09
    int 0x10
    jmp start

filetable:
    ; Load the filetable segment
    mov ax, 0x1000
    mov ds, ax
    mov es, ax

    ; Call the filetable code
    call 0x1000:0x0000
    jmp cmd_input


help:
    mov si, help_info
    call print_string
    jmp cmd_input

version:
    mov si,vinfo
    call print_string
    jmp cmd_input

phi_phi_cmd_dis:
    mov si,phi_msg1
    call print_string
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

;Data and variables
kernel_msg1 db ' _         _   __',10,13
kernel_msg2 db '|_) |_  o / \ (_  ',10,13
kernel_msg3 db '|   | | | \_/ __) ',10,13,0
help_msg db 10,13,'type phi -help for list of commands',10,13, 0
cmd_prompt db 10,13,'|>:', 0
reboot_cmd db 'reboot', 0
vinfo db 10,13,'0.0.1',0
dirs_cmd db 'dirs', 0
cls_cmd db 'cls', 0
help_cmd db 'phi -help', 0
version_cmd db 'phi -v', 0
phi_phi_cmd db 'phi -phi',0
invalid_cmd_msg db 10,13,'Invalid Command',10,13,0
help_info: db 10,13,'reboot - Reboot the system',10,13,'dirs - File table',10,13,\
'cls - Clear screen',10,13,'help - Help',10,13,'ret - Return to home', 10,13,'ls - List files'10,13'phi -v - Version', 0      
phi_msg1 db 10,13,\
            '       :===+@@@@+===-',10,13      
phi_msg2 db '     :-=+***@@@@*+++=-:',10,13      
phi_msg3 db '  =%@@%+:. .@@@@:  :=%@@#=',10,13   
phi_msg4 db ' %@@@#     .@@@@:     #@@@%',10,13  
phi_msg5 db '=@@@@-     .@@@@:     -@@@@+       PhiOS - 0.0.1',10,13 
phi_msg6 db '+@@@@:     .@@@@:     :@@@@+       Created by: SMF',10,13 
phi_msg7 db '.@@@@*     .@@@@:     +@@@@.',10,13 
phi_msg8 db ' .*@@@#-   .@@@@:   :*@@@*.',10,13  
phi_msg9 db '    :=+*#***@@@@#**##*=:',10,13          
phi_msg11 db '      :===+@@@@+===-',10,13,0        
                              

cmds: times 64 db 0 ; Make space for 64 characters of input

times 2048-($-$$) db 0

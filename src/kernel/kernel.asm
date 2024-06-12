org 0x0000 

; Set video mode
mov ah, 0x00
mov al, 0x03
int 0x10

; Set color
mov ah, 0x0B
mov bh, 0x00
mov bl, 0x01
int 0x10

; Print kernel messages
mov si, kernel_msg
call print_string
mov si, cmd_msg
call print_string
mov si, reboot_msg
call print_string
mov si, filetable_msg
call print_string

jmp cmd_input

;Error function 
error:
    mov si, invalid_cmd_msg
    call print_string
    jmp cmd_input

; Get input
cmd_input:
    mov di, cmds
    call get_input
    jmp $

; Get input function
get_input:
    mov ah, 0x00 ; Read from keyboard
    int 0x16 ; Keyboard interrupt

    mov ah, 0x0E ; Print character
    cmp al, 0x0D  ; If pressed enter
    je process_commands ; Jump execute commands
    int 0x10 ; Print character
    mov [di], al ; Store character
    inc di ; Increment character
    jmp get_input ; Loop

process_commands: 
    mov byte [di], 0 ; Move null terminator to end of input
    mov si, cmds ; Display input
    call execute_command ; Execute command
    jmp cmd_input ; Loop

execute_command:
    mov al, [si] ; Get character
    cmp al, 'r' ; Compare character to r
    je reboot ; If equal, jump to reboot 
    cmp al, 'R' ; Compare character to R
    je reboot ; If equal, jump to reboot
    cmp al, 'c' ; Compare character to c
    je cmd      ; If equal, jump to cmd    
    cmp al,'C'   ; Compare character to C
    je cmd       ; If equal, jump to cmd
    cmp al, 'f'  ; Compare character to f  
    je filetable  ; If equal, jump to filetable
    cmp al, 'F'
    je filetable
    jne error ; If not equal, jump to error

reboot:
    jmp 0xFFFF:0x0000 ; Reset vector

cmd: 
    mov si, cmd_prompt ; Display prompt
    call print_string
    jmp cmd_input ;Loop 

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

kernel_msg db '-------------------------------------------', 0xA, 0xD,0x20,0x20,0x20,0x20,'Welcome to PhiOS! ', 0xA, 0xD, \
'-------------------------------------------', 0xA, 0xD, 0
cmd_msg db 'Type cmd to run command prompt ', 0, 10, 13
invalid_cmd_msg db 10,13,'Invalid Command',10,13,0
cmd_prompt db 10,13,'Command prompt running',10,13,'|>', 0
reboot_msg db 10, 13, 'Press R to reboot', 0xA, 0xD, 0, 10, 13
filetable_msg db 'Press F for File Explorer', 10, 13, 0
cmds: db '', 0

times 512-($-$$) db 0

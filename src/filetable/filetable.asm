; File table header
header          db "File Table", 10, 13
version         db "v1.0", 0
total_entries   db 3                ; Updated number of entries

; Each file entry has:
; - 16 byte filename (null-terminated)
; - 2 byte memory segment (where to load)
; - 2 byte offset
; - 2 byte size in sectors

; First entry - Note program
file1_name      db "note", 0         ; Filename padded to 16 bytes
                times 12 db 0        ; Padding to make name 16 bytes
file1_segment   dw 0x3000            ; Load at segment 0x3000
file1_offset    dw 0x0000            ; Offset 0
file1_size      dw 2                 ; Size in sectors

; Second entry - Calc program
file2_name      db "calc", 0         ; Filename
                times 12 db 0        ; Padding
file2_segment   dw 0x4000            ; Load at segment 0x4000
file2_offset    dw 0x0000            ; Offset 0
file2_size      dw 3                 ; Size in sectors

; Third entry - Hello program
file3_name      db "hello", 0        ; Filename
                times 11 db 0        ; Padding to make name 16 bytes
file3_segment   dw 0x5000            ; Load at segment 0x5000
file3_offset    dw 0x0000            ; Offset 0
file3_size      dw 1                 ; Size in sectors

; Reserved space for more file entries
; We're just filling out the sector with zeros for now
times 4096-($-$$) db 0
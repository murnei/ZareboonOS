[org 0x7C00]

start:
    mov [boot_drive], dl 

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, message
print_loop:
    lodsb
    or al, al
    jz wait_key
    mov ah, 0x0E
    int 0x10
    jmp print_loop

wait_key:
    xor ah, ah
    int 0x16

load_kernel:
    xor ax, ax
    mov es, ax
    mov bx, 0x8000

    mov ah, 0x02
    mov al, 0x40
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02 
    mov dl, [boot_drive]

    int 0x13
    jc disk_error

    jmp 0x0000:0x8000

disk_error:
    mov al, 'E'
    mov ah, 0x0E
    int 0x10
    jmp $

boot_drive: db 0
message: db 'press any key...', 0

times 510-($-$$) db 0
dw 0xAA55

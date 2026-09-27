[bits 16]
[org 0x8000]

start:
    mov bx, 0x1000
    xor ax, ax
    mov es, ax

    mov ah, 02h
    mov al, 23
    mov ch, 0
    mov cl, 3
    mov dh, 0
    int 0x13
    jc disk_fail

    xor ax, ax
    mov es, ax
    mov di, 0x9000
    mov cx, 256
    push di
    rep stosw
    pop di

    mov dword [es:di], 'VBE2'

    mov ax, 0x4F00
    int 0x10
    cmp ax, 0x004F
    jne vbe_fail_0

    mov ax, [0x9000 + 0x0E]
    mov [mode_list_off], ax
    mov ax, [0x9000 + 0x10]
    mov [mode_list_seg], ax

    call find_mode
    cmp bx, 0xFFFF
    je vbe_fail_1

    mov [chosen_mode], bx

    mov ax, 0x4F01
    mov cx, bx
    mov di, 0x9200
    int 0x10
    cmp ax, 0x004F
    jne vbe_fail_1

    mov eax, [0x9200 + 0x28]
    mov [0x7000], eax

    mov ax, [0x9200 + 0x10]
    mov [0x7004], ax

    mov ax, [0x9200 + 0x12]
    mov [0x7006], ax

    mov ax, [0x9200 + 0x14]
    mov [0x7008], ax

    mov al, [0x9200 + 0x19]
    mov [0x700A], al

    mov ax, 0x4F02
    mov bx, [chosen_mode]
    or bx, 0x4000
    int 0x10
    cmp ax, 0x004F
    jne vbe_fail_2

    cli
    lgdt [GDT_Descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp dword 0x08:kernel_start

find_mode:
    push es
    push si
    push cx
    push dx

    mov ax, [mode_list_seg]
    mov es, ax
    mov si, [mode_list_off]

.next_mode:
    mov bx, [es:si]
    add si, 2

    cmp bx, 0xFFFF
    je .not_found

    push bx
    push es
    push si

    xor ax, ax
    mov es, ax
    mov ax, 0x4F01
    mov cx, bx
    push di
    mov di, 0x9400
    int 0x10
    pop di

    pop si
    pop es
    pop bx


    cmp ax, 0x004F
    jne .next_mode

    mov ax, [0x9400 + 0x12]
    cmp ax, 1024
    jne .next_mode

    mov ax, [0x9400 + 0x14]
    cmp ax, 768
    jne .next_mode

    mov al, [0x9400 + 0x19]
    cmp al, 32
    je .found

    cmp al, 24
    je .found

    jmp .next_mode

.found:
    pop dx
    pop cx
    pop si
    pop es
    ret

.not_found:
    mov bx, 0xFFFF
    pop dx
    pop cx
    pop si
    pop es
    ret



disk_fail:

    mov si, disk_error

    call print

    hlt



vbe_fail_0:

    mov si, vbe_error_0

    call print

    call print_hex_ax

    hlt



vbe_fail_1:

    mov si, vbe_error_1

    call print

    call print_hex_ax

    hlt



vbe_fail_2:

    mov si, vbe_error_2

    call print

    call print_hex_ax

    hlt



print_hex_ax:

    mov bx, ax

    mov cl, 4

.loop:

    rol bx, 4

    mov al, bl

    and al, 0x0F

    cmp al, 10

    jl .digit

    add al, 'A' - 10

.digit:

    add al, '0'

    mov ah, 0x0E

    int 0x10

    dec cl

    jnz .loop

    ret



print:

    mov ah, 0x0E

    mov al, [si]

    cmp al, 0

    jz return

    int 0x10

    inc si

    jmp print



return:

    ret



disk_error:  db "Disk failed", 0

vbe_error_0: db "VBE0 fail: ", 0

vbe_error_1: db "No mode found: ", 0

vbe_error_2: db "SetMode fail: ", 0



mode_list_off: dw 0

mode_list_seg: dw 0

chosen_mode:   dw 0



GDT_Start:

    null_descriptor:

        dq 0x0000000000000000

    code_descriptor:

        dw 0xFFFF, 0x0000

        db 0x00, 10011010b, 11001111b, 0x00

    data_descriptor:

        dw 0xFFFF, 0x0000

        db 0x00, 10010010b, 11001111b, 0x00

    user_code_descriptor:

        dw 0xFFFF, 0x0000

        db 0x00, 11111010b, 11001111b, 0x00

    user_data_descriptor:

        dw 0xFFFF, 0x0000

        db 0x00, 11110010b, 11001111b, 0x00

GDT_End:



GDT_Descriptor:
    dw GDT_End - GDT_Start - 1
    dd GDT_Start

[bits 32]
kernel_start:
    cli
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    mov ebx, 0x7000
    jmp 0x08:0x1000

ENTRY(_start)

SECTIONS {
    . = 0x10000;
    .text : {
        *(.text.entry)
        *(.text*)
    }
    .syscalls : {
        *(.syscalls)
    }

    .rodata : { *(.rodata*) }
    .data   : { *(.data*)   }
    .bss    : { *(.bss*) *(COMMON) }

    /DISCARD/ : { *(.note*) *(.comment) *(.eh_frame*) }
}

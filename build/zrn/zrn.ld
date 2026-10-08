ENTRY(main)

SECTIONS {
    . = 0x60000;

    .text : {
        KEEP(*(.text.entry))
        *(.text*)
    }

    .rodata : { *(.rodata*) }
    .data   : { *(.data*) }

    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    /DISCARD/ : { *(.note*) *(.comment) *(.eh_frame*) }
}

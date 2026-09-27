ENTRY(main)

SECTIONS {
    . = 0x60000;

    .zrn_header : {
        *(.zrn_header)
    }

    .text : {
        *(.text*)
    }
	.rodata : { *(.rodata*) }
    .data   : { *(.data*)   }
    .bss    : { *(.bss*) *(COMMON) }

    /DISCARD/ : { *(.note*) *(.comment) *(.eh_frame*) }
}

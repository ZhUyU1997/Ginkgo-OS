OUTPUT_ARCH( "riscv" )
ENTRY(_start)

SECTIONS
{
    . = 0x20000000;

    .text : {
        *libuser.a:libmain.S.o(.text)
        *(.text .text.*)
        . = ALIGN(0x1000);
        PROVIDE(etext = .);
    }

    .rodata : {
        . = ALIGN(16);
        *(.srodata .srodata.*) /* do not need to distinguish this from .rodata */
        . = ALIGN(16);
        *(.rodata .rodata.*)
    }

    .data : {
        . = ALIGN(16);
        *(.sdata .sdata.*) /* do not need to distinguish this from .data */
        . = ALIGN(16);
        *(.data .data.*)
    }

    /* move .bss to .data, avoid objcopy deleting it*/
    .data : {
        . = ALIGN(16);
        *(.sbss .sbss.*) /* do not need to distinguish this from .bss */
        . = ALIGN(16);
        *(.bss .bss.*)
    }

    PROVIDE(end = ALIGN(0x1000));
    PROVIDE(__heap_start = ALIGN(0x1000));
}

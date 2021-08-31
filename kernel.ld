OUTPUT_ARCH( "riscv" )
ENTRY( _entry )

SECTIONS
{
    /*
     * ensure that entry.S / _entry is at 0x80000000,
     * where qemu's -kernel jumps.
     */
    . = 0x80000000;

    .text : {
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

	.class.type ALIGN(8) :
	{
		PROVIDE(__class_type_start = .);
		KEEP(*(.class.type))
		PROVIDE(__class_type_end = .);
	}

	.class.table ALIGN(8) :
	{
		PROVIDE(__class_table_start = .);
		KEEP(*(.class.table))
		PROVIDE(__class_table_end = .);
	}

    .dtree ALIGN(8) :
	{
		PROVIDE(__dtree_start = .);
		KEEP(*(.dtree));
		PROVIDE(__dtree_end = .);
	}

    .bss : {
        PROVIDE(__bss_start = .);

        . = ALIGN(16);
        *(.sbss .sbss.*) /* do not need to distinguish this from .bss */
        . = ALIGN(16);
        *(.bss .bss.*)
        
        PROVIDE(__bss_end = .);
    }

    PROVIDE(__end = .);
}

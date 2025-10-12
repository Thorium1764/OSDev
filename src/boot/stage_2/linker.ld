/* stage2_linker.ld */
ENTRY(entry)    /* ASM stub that enters 32-bit mode and calls kmain */

SECTIONS
{
    /* Physical address where stage1 loads stage2 */
    . = 0x500;

    .text : {
        *(.text)    /* all code, including ASM stub */
    }

    .data : {
        *(.data)    /* initialized globals */
    }

    .bss : {
        *(.bss)     /* uninitialized globals */
        *(COMMON)
    }
}


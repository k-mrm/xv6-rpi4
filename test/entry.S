#include "aarch64.h" 
#include "memlayout.h"
#include "param.h"

        // qemu -kernel loads the kernel at 0x80000
        // and causes each CPU to jump there.
        // kernel.ld causes the following code to
        // be placed at 0x80000.
.section ".text"
.global _entry
_entry:
        mrs x1, mpidr_el1
        and x1, x1, #3
        cbz x1, swtch_el1
        b hang

hang:
        wfe
        b hang

        // switch EL2 to EL1
swtch_el1:
        mov x0, #(1 << 31) 
        orr x0, x0, #(1 << 1)
        msr hcr_el2, x0

        adr x1, entry
        msr elr_el2, x1

        mov x2, #0x3c5  // DAIF=0b1111, M=0b101(EL1H)
        msr spsr_el2, x2

        eret

entry:
        // clear .bss
        adrp x1, bss_start
        ldr w2, =bss_size
1:
        cbz w2, 2f
        str xzr, [x1], #8
        sub w2, w2, #1
        b 1b
2:
        // set up a stack for C.
        // stack0 is declared in start.c,
        // with a 4096-byte stack per CPU.
        // sp = stack0 + ((cpuid+1) * 4096)
        // cpuid = mpidr_el1 & 0xff
        ldr x0, =stack0
        mov x1, #1024*4
        mrs x2, mpidr_el1
        and x2, x2, #0x3
        add x2, x2, #1
        mul x1, x1, x2
        add x0, x0, x1
        mov sp, x0
        // jump to main()
        b main

        b .   // spin

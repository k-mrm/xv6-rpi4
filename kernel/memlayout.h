// Physical memory layout

// qemu -machine virt is set up like this,
// based on qemu's hw/arm/virt.c:
//
// 00000000 -- boot ROM, provided by qemu
// 08000000 -- GICv2
// 09000000 -- uart0 
// 0a000000 -- virtio disk 
// 40000000 -- boot ROM jumps here in machine mode
//             -kernel loads the kernel here
// unused RAM after 40000000.

// the kernel uses physical memory thus:
// 40000000 -- entry.S, then kernel text and data
// end -- start of kernel page allocation area
// PHYSTOP -- end RAM used by the kernel

#define REG(reg) ((volatile uint32 *)(reg))

#define EXTMEM    0x80000L        // Start PA of extended memory
#define PHYSTOP   (EXTMEM+128*1024*1024)     // PA of Top SDRAM

#define KERNBASE  0xffffff8000000000L     // First kernel virtual address
#define KERNLINK  (KERNBASE + EXTMEM)     // virtual address where kernel is linked

#define V2P(a) (((uint64)(a)) - KERNBASE)
#define P2V(a) ((void *)(((char *)(a)) + KERNBASE))

#define V2P_WO(x) ((x) - KERNBASE)    // same as V2P, but without casts
#define P2V_WO(x) ((x) + KERNBASE)    // same as P2V, but without casts

// one beyond the highest possible virtual address.
#define MAXVA (KERNBASE + (1ULL<<38))

// rpi4 peripheral base address
#define RPI4_PERI_BASE  0xfe000000L
#define RPI4_PERI_END   0x100000000L
#define RPI3_PERI_BASE  0x3f000000L
#define RPI3_PERI_END   0x40000000L

#define PPERI_BASE      RPI4_PERI_BASE
#define PPERI_END       RPI4_PERI_END
#define PERIPHERAL_BASE P2V_WO(PPERI_BASE)
//#define PERIPHERAL_BASE (PPERI_BASE)

// rpi4 gpio
#define GPIOBASE    (PERIPHERAL_BASE + 0x200000)

// rpi4 uart
#define UART(n)     (PERIPHERAL_BASE + 0x201000 + (n)*0x200)
#define UART0_IRQ   153

#define TIMER0_IRQ    27

// interrupt controller GICv2
#define GICDBASE     P2V_WO(0xff841000)
#define GICCBASE     P2V_WO(0xff842000)

// map kernel stacks beneath the trampoline,
// each surrounded by invalid guard pages.
#define KSTACK(p)   (MAXVA - ((p)+1) * 2*PGSIZE)

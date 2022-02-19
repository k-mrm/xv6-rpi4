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

// rpi4 peripheral base address
//#define PERIPHERAL_BASE (0xfe000000L)
#define PERIPHERAL_BASE (KERNBASE + 0x3f000000L)

// rpi4 gpio
#define GPIOBASE    (PERIPHERAL_BASE + 0x200000)

// rpi4 uart
#define UART(n)     (PERIPHERAL_BASE + 0x201000 + (n)*0x200)
#define UART0_IRQ   153

#define TIMER0_IRQ    27

// interrupt controller GICv2
#define GICDBASE     (0xff841000)
#define GICCBASE     (0xff842000)

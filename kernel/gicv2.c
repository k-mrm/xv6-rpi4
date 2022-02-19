#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "aarch64.h"
#include "defs.h"

//
// aarch64 Global Interrupt Controller (GICv2).
//

// GICD base
#define RegD(reg) ((volatile uint32 *)(GICDBASE + (reg)))
// GICC base
#define RegC(reg) ((volatile uint32 *)(GICCBASE + (reg)))

#define D_CTLR          0x0
#define D_TYPER         0x4
#define D_ISENABLER(n)  (0x100 + (uint64)(n) * 4)
#define D_ICENABLER(n)  (0x180 + (uint64)(n) * 4)
#define D_ISPENDR(n)    (0x200 + (uint64)(n) * 4)
#define D_ICPENDR(n)    (0x280 + (uint64)(n) * 4)
#define D_IPRIORITYR(n) (0x400 + (uint64)(n) * 4)
#define D_ITARGETSR(n)  (0x800 + (uint64)(n) * 4)
#define D_ICFGR(n)      (0xc00 + (uint64)(n) * 4)

#define C_CTLR  0x0 
#define C_PMR   0x4
#define C_IAR   0xc
#define C_EOIR  0x10
#define C_HPPIR 0x18
#define C_AIAR  0x20
#define C_AEOIR 0x24

static void gic_setup_ppi(uint32 intid);
static void gic_setup_spi(uint32 intid);

static void
giccinit()
{
  *RegC(C_CTLR) = 0;
  *RegC(C_PMR) = 0xff;
}

static void
gicdinit()
{
  *RegD(D_CTLR) = 0;
}

void
gicv2init()
{
  gic_setup_ppi(TIMER0_IRQ);
  gic_setup_spi(UART0_IRQ);
}

void
gicv2inithart()
{
  giccinit();
  gicdinit();

  *RegC(C_CTLR) |= 0x1;
  *RegD(D_CTLR) |= 0x1;
}

static void
gic_enable_int(uint32 intid)
{
  *RegD(D_ISENABLER(intid / 32)) |= 1 << (intid % 32);
}

static void
gic_clear_pending(uint32 intid)
{
  *RegD(D_ICPENDR(intid / 32)) |= 1 << (intid % 32);
}

static void
gic_set_prio0(uint32 intid)
{
  // set priority to 0
  *RegD(D_IPRIORITYR(intid / 4)) &= ~((uint32)0xff << (intid % 4 * 8));
}

static void
gic_set_target(uint32 intid, uint32 cpuid)
{
  uint32 i = *RegD(D_ITARGETSR(intid / 4));
  i &= ~((uint32)0xff << (intid % 4 * 8));
  *RegD(D_ITARGETSR(intid / 4)) = i | ((uint32)(1 << cpuid) << (intid % 4 * 8));
}

static void
gic_setup_ppi(uint32 intid)
{
  gic_set_prio0(intid);
  gic_clear_pending(intid);
  gic_enable_int(intid);
}

static void
gic_setup_spi(uint32 intid)
{
  gic_set_prio0(intid);

  // all interrupts are handled by cpu0　
  gic_set_target(intid, 0);

  gic_clear_pending(intid);
  gic_enable_int(intid);
}

// irq from iar
int
gic_iar_irq(uint32 iar)
{
  return iar & 0x3ff;
}

// interrupt acknowledge register:
// ask GIC what interrupt we should serve.
uint32
gic_iar()
{
  return *RegC(C_IAR);
}

// tell GIC we've served this IRQ.
void
gic_eoi(uint32 iar)
{
  *RegC(C_EOIR) = iar;
}


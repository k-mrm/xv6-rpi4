#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "aarch64.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

struct spinlock tickslock;
uint ticks;

// in trapvec.S, calls kerneltrap() or usertrap().
void alltraps();
void dump_tf(struct trapframe *tf);

extern int devintr();

void
trapinit(void)
{
  initlock(&tickslock, "time");
}

// set up to take exceptions and traps.
void
trapinithart(void)
{
  w_vbar_el1((uint64)alltraps);
  __sync_synchronize();
}

//
// handle an interrupt, exception, or system call from user space.
// called from uservec.S
//

void
userirq(void)
{
  struct proc *p = myproc();

  int which_dev = devintr();
  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2)
    yield();

  if(p->killed)
    exit(-1);

  intr_off();
}

void
usertrap(struct trapframe *tf)
{
  struct proc *p = myproc();

  uint64 ec = (r_esr_el1() >> 26) & 0x3f;
  if(ec == 21){
    // system call
    if(p->killed)
      exit(-1);

    intr_on();

    syscall();
  } else {
    printf("usertrap(): unexpected ec %p %p pid=%d\n", ec, r_esr_el1(), p->pid);
    printf("            elr=%p far=%p\n", r_elr_el1(), r_far_el1());
    p->killed = 1;
  }

  if(p->killed)
    exit(-1);

  intr_off();
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void 
kerneltrap()
{
  uint64 esr = r_esr_el1();
  
  if(intr_get() != 0)
    panic("kerneltrap: interrupts enabled");

  uint64 ec = (esr >> 26) & 0x3f;
  uint64 iss = esr & 0x1ffffff;
  printf("esr %p %p %p\n", esr, ec, iss);
  printf("elr=%p far=%p\n", r_elr_el1(), r_far_el1());
  panic("kerneltrap");
}

void
kernelirq()
{
  int which_dev = devintr();

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING)
    yield();
}

void
clockintr()
{
  acquire(&tickslock);
  ticks++;
  wakeup(&ticks);
  release(&tickslock);
}

// check if it's an external interrupt and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int
devintr()
{
  uint32 iar = gic_iar();
  int irq = gic_iar_irq(iar);
  int dev = 0;

  if(irq == UART0_IRQ){
    uartintr();
    dev = 1;
  } else if(irq == TIMER0_IRQ){
    if(cpuid() == 0)
      clockintr();
    timerintr();
    dev = 2;
  } else if(irq == 1023){
    // do nothing
  } else if(irq){
    printf("unexpected interrupt irq=%d\n", irq);
  }

  if(dev)
    gic_eoi(iar);

  return dev;
}

void
dump_tf(struct trapframe *tf)
{
  printf("trapframe dump: %p\n", tf);
  printf("x0 %p x1 %p x2 %p x3 %p\n", tf->x0, tf->x1, tf->x2, tf->x3);
  printf("x4 %p x5 %p x6 %p x7 %p\n", tf->x4, tf->x5, tf->x6, tf->x7);
  printf("x8 %p x9 %p x10 %p x11 %p\n", tf->x8, tf->x9, tf->x10, tf->x11);
  printf("x12 %p x13 %p x14 %p x15 %p\n", tf->x12, tf->x13, tf->x14, tf->x15);
  printf("x16 %p x17 %p x18 %p x19 %p\n", tf->x16, tf->x17, tf->x18, tf->x19);
  printf("x20 %p x21 %p x22 %p x23 %p\n", tf->x20, tf->x21, tf->x22, tf->x23);
  printf("x24 %p x25 %p x26 %p x27 %p\n", tf->x24, tf->x25, tf->x26, tf->x27);
  printf("x28 %p x29 %p x30 %p elr %p\n", tf->x28, tf->x29, tf->x30, tf->elr);
  printf("spsr %p sp %p\n", tf->spsr, tf->sp);
}

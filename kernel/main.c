#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "aarch64.h"
#include "defs.h"

#define  PSCI_CPUON   0xc4000003

volatile static int started = 0;
extern char end[];  // first address after kernel loaded from ELF file

void _entry(void);

// start() jumps here in EL1 on all CPUs.
void
main()
{
  if(cpuid() == 0){
    kinit1(end, (void*)(KERNLINK+2*1024*1024));  // physical page allocator
    kvminit();       // create kernel page table
    kvminithart();   // turn on paging
    kinit2((void*)(KERNLINK+2*1024*1024), P2V(PHYSTOP));
    consoleinit();
    printfinit();
    printf("\n");
    printf("xv6 kernel is booting\n");
    printf("\n");
    procinit();      // process table
    trapinit();      // trap vectors
    trapinithart();  // install trap vector
    gicv2init();     // set up interrupt controller
    gicv2inithart();
    timerinit();
    binit();         // buffer cache
    iinit();         // inode table
    fileinit();      // file table
    ramdiskinit();
    userinit();      // first user process
    __sync_synchronize();
    started = 1;
  } else {
    while(started == 0)
      ;
    __sync_synchronize();
    kvminithart();    // turn on paging
    printf("hart %d starting\n", cpuid());
    trapinithart();   // install trap vector
    gicv2inithart();
    timerinit();
  }

  scheduler();
}

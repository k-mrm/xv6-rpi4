#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "aarch64.h"
#include "defs.h"

volatile static int started = 0;
extern char end[];  // first address after kernel loaded from ELF file

void _entry(void);

// start() jumps here in EL1 on all CPUs.
void
main()
{
  if(cpuid() == 0){
    consoleinit();
    trapinit();      // trap vectors
    trapinithart();  // install trap vector
    //char *s = "chompppppppppppp";
    printf("%p111", mycpu());
    printfinit();
    printf("%p222", mycpu());
    kinit1(end, (void*)SECTROUNDUP(KERNLINK));  // physical page allocator
    printf("%p33", mycpu());
    kvminit();       // create kernel page table
    printf("%paa", mycpu());
    kvminithart();   // turn on paging
    printf("%pcc", mycpu());
    kinit2((void*)SECTROUNDUP(KERNLINK), P2V(PHYSTOP));
    printf("%pcsex", mycpu());
    printf("\n");
    printf("xv6 kernel is booting\n");
    printf("\n");
    procinit();      // process table
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

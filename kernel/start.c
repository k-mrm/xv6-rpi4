#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "aarch64.h"
#include "defs.h"

void _entry();
void main();
extern char end[];

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

void
cpu1_wakeup(uint64 entry)
{
  *(volatile uint64 *)0xe0 = entry;
  __sync_synchronize();
  asm volatile("sev");
}

void
cpu2_wakeup(uint64 entry)
{
  *(volatile uint64 *)0xe8 = entry;
  __sync_synchronize();
  asm volatile("sev");
}


void
cpu3_wakeup(uint64 entry)
{
  *(volatile uint64 *)0xf0 = entry;
  __sync_synchronize();
  asm volatile("sev");
}

void
start()
{
  main();
}

__attribute__((aligned(PGSIZE))) pte_t l1entrypgt[512];
__attribute__((aligned(PGSIZE))) pte_t l2entrypgt0[512];
__attribute__((aligned(PGSIZE))) pte_t l1kpgt[512];
__attribute__((aligned(PGSIZE))) pte_t l2kpgt[512];
__attribute__((aligned(PGSIZE))) pte_t l2kpgt1[512];

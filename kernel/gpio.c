#include "types.h"
#include "param.h"
#include "aarch64.h"
#include "memlayout.h"
#include "gpio.h"
#include "defs.h"

#define GPFSEL(n) (GPIOBASE + 0x4 * (n))
#define GPSET(n)  (GPIOBASE + 0x1c + 0x4 * (n))
#define GPCLR(n)  (GPIOBASE + 0x28 + 0x4 * (n))

void
gpio_set(int pin)
{
  int ch = pin / 32;
  int n = pin % 32;

  *REG(GPSET(ch)) |= 1 << n;
}

void
gpio_clr(int pin)
{
  int ch = pin / 32;
  int n = pin % 32;

  *REG(GPCLR(ch)) |= 1 << n;
}

void
gpio_pull(int pin, enum pullmode m)
{
  ;
}

void
set_pinmode(int pin, enum pinmode mode)
{
  int ch = pin / 10;
  int n = pin % 10 * 3;

  *REG(GPFSEL(ch)) &= ~(7 << n);
  *REG(GPFSEL(ch)) |= mode << n;
}

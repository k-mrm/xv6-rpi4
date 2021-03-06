//
// low-level driver routines for pl011 UART.
//

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "aarch64.h"
#include "defs.h"
#include "gpio.h"

// pl011
#define DR(n)    (UART(n) + 0x00)
#define FR(n)    (UART(n) + 0x18)
#define FR_RXFE (1<<4)  // recieve fifo empty
#define FR_TXFF (1<<5)  // transmit fifo full
#define FR_RXFF (1<<6)  // recieve fifo full
#define FR_TXFE (1<<7)  // transmit fifo empty
#define IBRD(n)  (UART(n) + 0x24)
#define FBRD(n)  (UART(n) + 0x28)
#define LCRH(n)  (UART(n) + 0x2c)
#define LCRH_FEN        (1<<4)
#define LCRH_WLEN_8BIT  (3<<5)
#define CR(n)    (UART(n) + 0x30)
#define IMSC(n)  (UART(n) + 0x38)
#define INT_RX_ENABLE (1<<4)
#define INT_TX_ENABLE (1<<5)
#define ICR(n)   (UART(n) + 0x44)

#define UART_FREQ 48000000ull

// the UART control registers.
// pl011

#define ReadReg(reg)     (*(REG(reg)))
#define WriteReg(reg, v) (*(REG(reg)) = (uint32)(v))

// the transmit output buffer.
#define UART_TX_BUF_SIZE 32
char uart_tx_buf[UART_TX_BUF_SIZE];
uint64 uart_tx_w; // write next to uart_tx_buf[uart_tx_w % UART_TX_BUF_SIZE]
uint64 uart_tx_r; // read next from uart_tx_buf[uart_tx_r % UART_TX_BUF_SIZE]

extern volatile int panicked; // from printf.c

void
set_uart_baudrate(int n, uint64 baud)
{
  uint64 bauddiv = (UART_FREQ * 1000) / (16 * baud);
  uint64 ibrd = bauddiv / 1000;
  uint64 fbrd = ((bauddiv - ibrd * 1000) * 64 + 500) / 1000;

  WriteReg(IBRD(n), (uint32)ibrd);
  WriteReg(FBRD(n), (uint32)fbrd);
}

void
uartinit(int n)
{
  // disable uart
  WriteReg(CR(n), 0);

  // disable interrupts.
  WriteReg(IMSC(n), 0);

  // set baudrate.
  set_uart_baudrate(n, 115200);

  // enable FIFOs.
  // set word length to 8 bits, no parity.
  WriteReg(LCRH(n), LCRH_FEN | LCRH_WLEN_8BIT);

  // enable RXE, TXE and enable uart.
  WriteReg(CR(n), 0x301);

  // enable transmit and receive interrupts.
  WriteReg(IMSC(n), INT_RX_ENABLE | INT_TX_ENABLE);

  set_pinmode(14, ALT0);
  set_pinmode(15, ALT0);
}

// add a character to the output buffer and tell the
// UART to start sending if it isn't already.
// blocks if the output buffer is full.
// because it may block, it can't be called
// from interrupts; it's only suitable for use
// by write().

// alternate version of uartputc() that doesn't 
// use interrupts, for use by kernel printf() and
// to echo characters. it spins waiting for the uart's
// output register to be empty.
void
uartputc_sync(int c)
{
  // wait for ... TODO: comment */
  while(ReadReg(FR(0)) & FR_TXFF)
    ;
  WriteReg(DR(0), c);
}

// read one input character from the UART.
// return -1 if none is waiting.
int
uartgetc(void)
{
  if(ReadReg(FR(0)) & FR_RXFE)
    return -1;
  else
    return ReadReg(DR(0));
}

// handle a uart interrupt, raised because input has
// arrived, or the uart is ready for more output, or
// both. called from trap.c.
void
uartintr(void)
{
  // read and process incoming characters.
  while(1){
    int c = uartgetc();
    if(c == -1)
      break;
  }

  // clear transmit and receive interrupts.
  WriteReg(ICR(0), INT_RX_ENABLE|INT_TX_ENABLE);
}

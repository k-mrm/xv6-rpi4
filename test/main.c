#include "types.h"
#include "aarch64.h"
#include "defs.h"

int main(void) {
  uartinit(0);

  char *s = "hello";
  while(*s) {
    uartputc_sync(*s++);
  }

  for(;;)
    ;

  return 0;
}

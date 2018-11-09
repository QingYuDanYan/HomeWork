#include <stdio.h>
#include <stdlib.h>
#include "monitor/expr.h"

int init_monitor(int, char *[]);
void ui_mainloop(int);

int main(int argc, char *argv[]) {
  if (argc == 2 && strcmp(argv[1], "expr_test") == 0) {
    printf("expr_test begin\n");
    return 0;
  }  

  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);

  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);

  return 0;
}

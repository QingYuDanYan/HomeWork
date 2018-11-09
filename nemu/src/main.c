#include <stdio.h>
#include <stdlib.h>
#include "monitor/expr.h"
#include <readline/readline.h>

int init_monitor(int, char *[]);
void ui_mainloop(int);

int main(int argc, char *argv[]) {
  
  int is_batch_mode = init_monitor(argc, argv);

  //if (argc == 2 && strcmp(argv[1], "expr_test") == 0) {
    printf("expr_test begin\n");
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen("./tools/gen-expr/expr_testcase", "r");
    if (fp == NULL) Assert(0, "expr_testcase is NULL\n");

    while ((read = getline(&line, &len, fp)) != -1) {
      char *pos;
      if ((pos = strchr(line, '\n')) != NULL)
        *pos = '\0';
      char *arg1 = strtok(line, " ");
      char *arg2 = strtok(NULL, "");
      bool *success = false;
     // arg2  = readline("(nemu) ");      
     // printf("arg1: %s arg2: %s\n", arg1, arg2);
      uint32_t res = expr(arg2, success);
      uint32_t ret = atoi(arg1);

      if (res != ret) {
        Assert(0, "%s mismatch\n", line);
      }
      else {
        printf("expr %s = %d\n", arg2, res);
      }
    }

    free(line);
    fclose(fp);

    return 0;
  //}  

  /* Initialize the monitor. */

  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);

  return 0;
}

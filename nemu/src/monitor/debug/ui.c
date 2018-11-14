#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_expr_test(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "single step si [N]", cmd_si },
  { "cmd", "display info SUBCMD", cmd_info },
  { "x", "print memory x N EXPR", cmd_x },
  { "p", "expr evaluate", cmd_p  },
  { "w", "watch expr", cmd_w },
  { "d", "delete watch point", cmd_d },
  { "expr_test", "expr test", cmd_expr_test  },
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))
static int cmd_w (char *args) {
  char *arg = strtok(NULL, "");
  if (arg == NULL) {
    printf("Wrong usage, please use help command\n ");
  }

  WP* wp = new_wp();
  strcpy(wp->expr, arg);
  printf("Watchpoint %d: %s\n", wp->NO, wp->expr);

  return 0;
}

static int cmd_d (char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("Wrong usage, please use help command\n ");
  }
  
  return 0; 
}

static int cmd_expr_test(char *args) { 
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
    uint32_t res = expr(arg2, success);
    uint32_t ret = atoi(arg1);

    if (res != ret) {
      Assert(0, "%d = %s, but expr = %d \n", ret, arg2, res);
    }
    else {
      printf("expr %s = %d\n", arg2, res);
    }
  }

  free(line);
  fclose(fp);

  return 0;

}

static int cmd_p(char *args) {
  char *arg = strtok(NULL, "");
  if (arg == NULL){
    printf("Wrong Format\n");
  }
  bool *success = false;
  uint32_t res = expr(arg, success);
  printf("expr value: %d\n", res);
  return 0;
}

static int cmd_x(char *args) {
  char *arg1 = strtok(NULL, " ");
  char *arg2 = strtok(NULL, " ");
  char *arg3 = strtok(NULL, " ");

  if (arg1 == NULL || arg3 != NULL) {
    printf("Wrong Format\n");
  }
  else if (arg2 == NULL) {
    
  }
  else {

  }
  
  printf("%s %s %s\n", arg1, arg2, arg3);

  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  
  if (arg == NULL) {
    printf("print registers and watchpoints\n");
  }
  else if (*arg == 'r') {
    for (int i=0; i < 8; i ++){
      printf("%-14s 0x%x    %d\n", reg_name(i, 4), reg_l(i), reg_l(i));
    }
  }
  else if (*arg == 'w') {
    printf("watch point unfinished\n");
  }
  else {
    printf("Unkown Command '%s'\n", arg);
  }

  return 0;
}

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  
  if (arg == NULL) {
    cpu_exec(1);
  }
  else {
    uint64_t step = atol(arg);
    cpu_exec(step);
  }

  return 0;
}

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

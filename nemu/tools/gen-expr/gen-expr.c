#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536];

static inline void inline gen_num() {
  uint32_t num = rand() % 50;
  char buffer[10];
  if (rand() % 2 == 0){
    sprintf(buffer, "%d", num);
  }
  else {
    strcpy(buffer, "0x");
    sprintf(buffer+2, "%x ", num);
  }
  strcat(buf, buffer);
}

static inline void gen_rand_op() {
  char op;
  switch (rand() % 4) {
    case 0: op = '+'; break;
    case 1: op = '-'; break;
    case 2: op = '*'; break;
    default: op = '/'; break;
  }
  char buffer[2];
  buffer[0] = op;
  buffer[1] = '\0';

  strcat(buf, buffer);
}

static inline void gen_rand_space() {
  return;
  char buffer[10];
  int cnt = 0;

  for(int i = 0; i < 10; i++) {
    if (rand() % 2 == 0) buffer[cnt++] = ' ';
  }
  buffer[cnt] = '\0';
  
  strcat(buf, buffer);

}

static inline void gen_rand_expr() {
  switch (rand()%5) {
    case 0: gen_num(); break;
    case 1: gen_num(); break;
    case 2: gen_num(); break;
    case 3: strcat(buf, "("); gen_rand_space(); gen_rand_expr(); strcat(buf, ")"); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_space(); gen_rand_expr(); break;
  }
}

static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"#include <signal.h>\n"
"#include <stdlib.h>\n"
"void sighandler(int signum) {"
"  exit(2);"
"}"

"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0;"
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  buf[0] = '\0';
  for (i = 0; i < loop; i ++) {
    memset(buf, '\0', sizeof(buf));
    gen_rand_expr();
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen(".code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);
    int ret = system("gcc .code.c -o .expr");
    if (ret != 0) continue;

    //ret = system("./.expr");
    //if (ret != 0) continue;

    fp = popen("./.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}

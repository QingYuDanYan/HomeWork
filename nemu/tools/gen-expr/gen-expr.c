#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

static inline char gen_rand_op(){
  switch (rand()%4) {
    case 0: return '+';
    case 1: return '-';
    case 2: return '*';
    default: return '/';
  }
}


// this should be enough
static char buf[65536];
static inline void gen_rand_expr() {
  switch (rand()%5) {
    case 0: return rand();
    case 1: return '(' + gen_rand_expr() + ')';
    case 2: return '+' + gen_rand_expr();
    case 3: return '-' + gen_rand_expr();
    default: return gen_rand_expr() + gen_rand_op() + gen_rand_expr();
  }

  buf[0] = '\0';
}

static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen(".code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc .code.c -o .expr");
    if (ret != 0) continue;

    fp = popen("./.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}

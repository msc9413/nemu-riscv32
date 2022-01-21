#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

// this should be enough
#define BUF_SIZE 65536
#define BLANK_NUM 4
static char buf[BUF_SIZE];
static uint32_t buf_index = 0;

static inline void gen(char c);

uint32_t choose(uint32_t n) {
  //printf("begin choose %d\n", buf_index);
  return rand() % n;
}

// 随机生成小于n个空格
static inline void gen_blank() {
  int blank_num = choose(BLANK_NUM);
  for (int i = 0; i < blank_num; i++) {
    gen(' ');
  }
}

static inline void gen(char c) {

  if (buf_index < BUF_SIZE) buf[buf_index++] = c;
  else return ; 

  //printf("gen %c\n", c);
}

static inline void gen_num() {
  gen_blank(); 

  uint32_t num = choose(100);
  char num_str[8] = {0};
  sprintf(num_str, "%u", num);
  if (buf_index + strlen(num_str) < BUF_SIZE) strncpy(buf + buf_index, num_str, strlen(num_str));
  else return ;
  buf_index += strlen(num_str);

  //printf("gen %u\n", num);

  gen_blank(); 
}

static inline void gen_rand_op() {
  gen_blank(); 

  switch (choose(4)) {
    case 0: gen('+'); break;
    case 1: gen('-'); break;
    case 2: gen('*'); break;
    case 3: gen('/'); break;
    default: assert(0); break;
  }

  gen_blank(); 
}

static inline void gen_rand_expr() {

  switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
  
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
    //printf("begin gen expr\n");

    buf_index = 0;
    gen_rand_expr();
    if (buf_index >= BUF_SIZE - 1 || 
      (buf[buf_index-1] != ')' && !isdigit(buf[buf_index - 1]))
    ) {
      i--;
      continue;
    }
    buf[buf_index++] = '\0';

    // strncpy(buf, "12 / 0", 128);
    //printf("end gen expr: %s\n", buf);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}

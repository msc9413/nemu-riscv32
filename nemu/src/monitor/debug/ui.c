#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
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

static int cmd_si(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int n = 1;  // 缺省为1

  if (arg != NULL) {
    n = atoi(arg);
  } 

  // 单步执行n次
  cpu_exec(n);

  return 0;
}

static int cmd_info(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");

  if (arg == NULL) {
    /* no argument given */
    printf("Input subcmd 'r' or 'w'\n");
  } else if (strcmp(arg, "r") == 0){  
    // 打印寄存器状态
    extern void isa_reg_display(void);
    isa_reg_display();
  } else if (strcmp(arg, "w") == 0){  
    // 打印监视点信息
    watchpoint_display();
  } else {
    printf("Unknown subcmd '%s'\n", arg);
  }

  return 0;
}

// 扫描内存
static int cmd_x(char *args) {
  /* extract the argument */
  char *N = strtok(NULL, " ");
  char *e = strtok(NULL, "\n");
  if (N == NULL || e == NULL) {
    // 参数错误
    printf("A syntax error in expression\n");
  } else {
    int n = atoi(N);

    // 求出表達式的值
    bool success;
    paddr_t base_addr = expr(e, &success);
    if (!success) {
      printf("Error expression!\n");
      return 0;
    }

    // 输出结果
    int i;
    for (i = 0; i < n; i++) {
      if (i % 4 == 0) {
        if (i != 0) printf("\n");
        printf("%#x:\t", base_addr);
      }
      printf("%#x\t", paddr_read(base_addr, 4));
      base_addr += 4;
    }
    printf("\n");
  }
  return 0;
}

static int cmd_d(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");

  if (arg == NULL) {
    /* no argument given */
    printf("Input the number of watchpoint\n");
  } else {
    uint32_t num = atoi(arg);
    free_wp(num);
    printf("Watchpoint %u have been removed\n", num);
  }

  return 0;
}

static int cmd_p(char *args) {
  /* extract the first argument */
  char *e = strtok(NULL, "\n");
  // 求出表達式的值
  bool success;
  uint32_t result = expr(e, &success);
  if (!success) {
    printf("Error expression!\n");
    return 0;
  }
  printf("%s = %d\n", e, result);

  return 0;
}

static int cmd_w(char *args) {
  /* extract the first argument */
  char *e = strtok(NULL, "\n");
  
  WP* wp = new_wp(e);
  assert(wp);
  printf("Watchpoint %u: %s\n", wp->NO, e);

  return 0;
}


static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "'si [N]'. Execute N instructions in a single step. The default value of N is 1", cmd_si},
  { "info", "Print register state", cmd_info},
  { "p", "'p EXPR'. Evaluate the expression EXPR", cmd_p},
  { "x", "'x N EXPR'. Scanning memory", cmd_x},
  { "w", "'w EXPR'. Suspends program execution when the value of the expression EXPR changes", cmd_w},
  { "d", "'d [N]'. Delete the number watchpoint", cmd_d},
  
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

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

  // 测试表达式结果
  // FILE *fp = fopen("/home/hust/Desktop/ics2019/nemu/tools/gen-expr/input", "r");
  // if (fp != NULL) {
  //   printf("Start testing the expression results:\n");
  //   uint32_t result, i = 1, success_num = 0;
  //   char e[65536] = {0};
  //   while (fscanf(fp, "%u %[^\n]s\n", &result, e) != EOF) {
  //     //printf("%u, %s--\n", result, e);
  //     bool success = true;
  //     extern uint32_t expr(char *, bool *);
  //     uint32_t ret = expr(e, &success);
  //     if (success == true && result == ret) {
  //       printf("Expression number %u\t evaluates correctly\n", i);
  //       success_num++;
  //     } else {
  //       printf("Expression error %u\n", i);
  //     }
  //     i++;
  //     e[0] = '\0';
  //   }
  //   printf("There are %u expressions. %u expressions were calculated correctly!\n", i - 1, success_num);
  //   fclose(fp);
  // }
  

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

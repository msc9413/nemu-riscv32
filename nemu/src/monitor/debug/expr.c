#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DECINT, 
  TK_HEXINT, TK_REG, TK_DEREF, TK_UNEQ, 
  TK_LOGICAND

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {"0x[0-9a-fA-F]+", TK_HEXINT},   // hexadecimal number
  {"[1-9][0-9]*|0", TK_DECINT},   //十进制整数 Decimal integer
  {"-", '-'},           // subtraction
  {"\\*", '*'},         // multiply
  {"/", '/'},           // divide
  {"\\(", '('},         // Left parenthesis
  {"\\)", ')'},         // Right parenthesis
  {"!=", TK_UNEQ},         // unequal
  {"\\$[a-zA-Z_][a-zA-Z0-9_]*", TK_REG},         // reg_name
  {"&&", TK_LOGICAND},         // logic and
  
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ}         // equal
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

#define TOKENS_SIZE 65536
static Token tokens[TOKENS_SIZE] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        assert(nr_token < TOKENS_SIZE);
        tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type) {
          // 数字和寄存器 需要记录下来
          case TK_REG:
          case TK_HEXINT:
          case TK_DECINT: 
            assert(substr_len < 32);
            strncpy(tokens[nr_token].str, substr_start, substr_len); 
            tokens[nr_token].str[substr_len] = '\0';
            break;
          default: break;
        }
        if (rules[i].token_type != TK_NOTYPE) ++nr_token;

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q, bool *success) {
  // 判断是不是Bad expression， 不包括() 
  int cnt = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') cnt++;
    else if (tokens[i].type == ')') cnt--;
    if (cnt < 0) break;
  }
  if (cnt != 0) {
    *success = false;
    return false;
  }
  // 判断表达式是否被一对匹配的括号包围着
  if (tokens[p].type != '(' || tokens[q].type != ')') return false;
  cnt = 0;
  for (int i = p + 1; i < q; i++) {
    if (tokens[i].type == '(') cnt++;
    else if (tokens[i].type == ')') cnt--;
    if (cnt < 0) break;
  }
  return cnt == 0;
}

int index_main_op(int p, int q) {
  int op = p, cnt;
  for (int i = p; i <= q; i++) {
    switch (tokens[i].type) {
      // 跳过括号中的内容
      case '(':
        cnt = 1;
        while (i <= q && cnt != 0) {
          i++;
          if (tokens[i].type == '(') cnt++;
          else if (tokens[i].type == ')') cnt--;
        }
        break;
      // &&
      case TK_LOGICAND:  
        op = i; 
        break;
      // ==  !=
      case TK_EQ:       
      case TK_UNEQ:  
        if (tokens[op].type != TK_LOGICAND) {
          op = i; 
        }
        break;
      case '+':
      case '-':
        if (tokens[op].type != TK_LOGICAND 
          && tokens[op].type != TK_EQ
          && tokens[op].type != TK_UNEQ
        ) {
          op = i;
        }
        break;

      case '*':
      case '/':
        if (tokens[op].type != TK_LOGICAND 
          && tokens[op].type != TK_EQ
          && tokens[op].type != TK_UNEQ
          && tokens[op].type != '+' 
          && tokens[op].type != '-'
        ) {
          op = i;
        } 
        break;

      case TK_DEREF:   
        if (tokens[op].type != TK_LOGICAND 
          && tokens[op].type != TK_EQ
          && tokens[op].type != TK_UNEQ
          && tokens[op].type != '+' 
          && tokens[op].type != '-'
          && tokens[op].type != '*' 
          && tokens[op].type != '/'
        ) {
          op = i;
        } 
        break;

      default:
        break;
    }
  }
  return op;
}

uint32_t isa_reg_str2val(const char *s, bool *success);
uint32_t eval(int p, int q, bool *success) {
  if (p > q) {
    /* Bad expression */
    *success = false;
    return 0;
  } else if (p == q) {
    // 单个token，数字 或者是 寄存器
    switch (tokens[p].type) {
      case TK_DECINT: return atoi(tokens[p].str);
      case TK_HEXINT: return strtol(tokens[p].str, NULL, 16);
      case TK_REG:    return isa_reg_str2val(tokens[p].str + 1, success);
      default:        *success = false; return 0; 
    }
  } else if (check_parentheses(p, q, success) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, success);
  } else if (*success == false) {
    // 不被括号包围 因为括号不匹配
    return 0;
  } else {
    int op = index_main_op(p, q);
    // 当前token为指针
    if (tokens[op].type == TK_DEREF) {
      uint32_t addr = eval(op + 1, q, success);
      if (*success == false) return 0;
      return paddr_read(addr, 4);
    }

    uint32_t val1 = eval(p, op - 1, success);
    uint32_t val2 = eval(op + 1, q, success);
    if (*success == false) return 0;

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': 
        if (val2 == 0) {
          *success = false;
          return 0;
        } else {
          return val1 / val2;
        }
      case TK_EQ:       return val1 == val2;
      case TK_UNEQ:     return val1 != val2;
      case TK_LOGICAND: return val1 && val2;
      default: assert(0);
    }
  }
  return 0;
}

uint32_t expr(char *e, bool *success) {
  *success = true;
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */

  for (int i = 0; i < nr_token; i ++) {
    if (tokens[i].type == '*' && (i == 0 
      || tokens[i - 1].type == '('
      || tokens[i - 1].type == '+'
      || tokens[i - 1].type == '-'
      || tokens[i - 1].type == '*'
      || tokens[i - 1].type == '/'
      || tokens[i - 1].type == TK_EQ
      || tokens[i - 1].type == TK_UNEQ
      || tokens[i - 1].type == TK_LOGICAND
    )) {
      tokens[i].type = TK_DEREF;
    }
  }

  return eval(0, nr_token - 1, success);

}

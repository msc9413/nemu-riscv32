#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].e[0] = '\0';
    wp_pool[i].hit_times = 0;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

// 从free_链表中返回一个空闲的监视点结构
WP* new_wp(char* expression) {
  if (free_ == NULL) {
    assert(0);
    return NULL;
  } 

  // 从free_取出一个WP
  WP* ret = free_;
  free_ = free_->next;

  // 将WP插入head开头
  ret->next = head;
  head = ret;

  // 设置监视点表达式
  int expr_len = strlen(expression);
  assert(expr_len < sizeof(ret->e));
  strncpy(ret->e, expression, expr_len);
  ret->e[expr_len] = '\0';

  bool success;
  ret->old_value = expr(expression, &success);
  assert(success);

  ret->hit_times = 0;

  return ret;
}

// 将wp从head归还到free_链表中
void free_wp(uint32_t num) {
  if (head == NULL) {
    assert(0);
    return ;
  } 
  WP* wp = NULL;
  if (head->NO == num) {
    wp = head;
    head = head->next;
    wp->next = free_;
    free_ = wp;
  } else {
    WP* pre = head;
    while (pre->next->NO != num) {
      if (pre->next == NULL) {
        printf("Don't find number %u watchpoint\n", num);
        return ;
      }
      pre = pre->next;
    }
    wp = pre->next;
    pre->next = pre->next->next;

    wp->next = free_;
    free_ = wp;
  }
  return ;
  // if (wp == head) {
  //   head = head->next;
  //   wp->next = free_;
  //   free_ = wp;
  // } else {
  //   WP* pre = head;
  //   while (pre->next != wp) {
  //     assert(pre->next != NULL);
  //     pre = pre->next;
  //   }
  //   pre->next = pre->next->next;

  //   wp->next = free_;
  //   free_ = wp;
  // }
  // return ;
}

// 打印使用中的监视点信息
void watchpoint_display() {
  printf("Num\tType\t\tWhat\n");
  WP* p = head;
  while (p) {
    printf("%u\twatchpoint\t%s\n",p->NO, p->e);
    if (p->hit_times)
      printf("\tbreakpoint already hit %u time\n", p->hit_times);
    p = p->next;
  }
}

// 检查监视点是否变化
bool check_all_watchpoints() {
  WP* p = head;
  bool is_change = false;
  while (p) {
    bool success;
    uint32_t new_value = expr(p->e, &success);
    assert(success);  

    if (new_value != p->old_value) {
      printf("Watchpoint %u: %s\n", p->NO, p->e);
      printf("Old value = %u\nNew value = %u\n\n", p->old_value, new_value);
      p->hit_times ++;
      p->old_value = new_value;
      is_change = true;
    }
    p = p->next;
  }
  return is_change;
}


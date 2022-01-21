#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char e[65535];      // 保存表达式
  uint32_t old_value; // 记录旧值
  uint32_t hit_times; // 命中次数
} WP;
WP* new_wp(char* );
void free_wp(uint32_t );
void watchpoint_display();
bool check_all_watchpoints();
#endif

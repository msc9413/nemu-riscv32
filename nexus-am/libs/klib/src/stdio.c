#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define STR_LEN 256

int printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt); // 指向可变参数表中的第一个参数 
  char outBuf[STR_LEN] = {'\0'};
  int length = vsprintf(outBuf, fmt, args); 
  for (size_t i = 0; outBuf[i]; i++) {
    _putc(outBuf[i]);
  }
  va_end(args);
  return length;
}

int is_digit(char c) {
  return c >= '0' && c <= '9';
}

int add_string(char *out, int out_len, char *str, int width, int is_left) {
  if (is_left) {        // 左对齐在尾部填充空白字符
    while (*str) {
      out[out_len++] = *(str++);
      width--;
    }
    while (width-- > 0) {
      out[out_len++] = ' ';
    }
  } else {              // 右对齐在头部填充相应字符
    int str_len = strlen(str);
    for (int i = 0; i < width - str_len; i++) {
      out[out_len++] = ' ';
    }
    while (*str) {
      out[out_len++] = *(str++);
    }
  }
  return out_len;
}

int add_char(char *out, int out_len, char c, int width, int is_left) {
  char str[2] = {0};
  str[0] = c;
  return add_string(out, out_len, str, width, is_left);
}

int add_number(char *out, int out_len, unsigned int num, int is_neg, int width, int is_left, char fill_char, int scale) {
  // 目前只支持10和16进制
  assert(scale == 10 || scale == 16);
  
  char str_number[STR_LEN] = {0};
  int str_index = 0;
  if (num == 0) {
    str_number[str_index++] = '0';
  }

  while (num) {
    if (num % scale < 10) {
      str_number[str_index++] = num % scale + '0';
    } else {
      str_number[str_index++] = num % scale - 10 + 'a';
    }
    num /= scale;
  }

  if (is_neg) {
    str_number[str_index++] = '-';
  }

  if (is_left) fill_char = ' ';

  // 负数 右对齐 0填充 十进制 需要将符号移到最开始位置 然后中间填充字符
  if (is_neg && !is_left && fill_char == '0' && scale == 10) {
    str_index--;
    while (str_index < width - 1) {
      str_number[str_index++] = fill_char;
    }
    str_number[str_index++] = '-';
  } else {
    while (str_index < width) {
      str_number[str_index++] = fill_char;
    }
  }

  // 将结果反转 输出到out中
  while (--str_index >= 0) {
    out[out_len++] = str_number[str_index];
  }

  return out_len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  // char* str[STR_LEN];   // 暂存%输出的内容
  int out_len = 0;      // 记录输出数组长度
  for (const char *p = fmt; *p; p++) {
    // 将非%格式化输出的内容 直接拷贝过去
    while (*p != '\0' && *p != '%') {
      out[out_len++] = *(p++);
    }

    // 遍历结束
    if (*p == '\0') {
      break;
    }

    ++p;  // 跳过当前%

    int width = 0;     // 输出宽度
    int is_left = 0;   // 是否左对齐
    int is_neg = 0;    // 是否为负数
    char fill_char = ' '; // 默认用‘ ’空格填充右对其多出来的内容
    
    if (*p == '0') {        // 以0填充
      fill_char = '0';
      p++;
    } else if (*p == '-') { // 指定为左对齐
      is_left = 1;
      p++;
    }

    // 计算格式化宽度
    while (is_digit(*p)) {
      width = width * 10 + *p - '0';
      p++;
    }

    // 用于保存参数
    int arg_num;
    char *arg_s;
    char agr_c;
    // 临时数组长度
    // int str_len = 0;
    switch (*p) {
      case 'd':
        arg_num = va_arg(ap, int);
        if (arg_num < 0) {
          is_neg = 1;
          arg_num = -arg_num;
        }
        out_len = add_number(out, out_len, arg_num, is_neg, width, is_left, fill_char, 10);
        break;

      case 'u':
        arg_num = va_arg(ap, int);
        out_len = add_number(out, out_len, arg_num, 0, width, is_left, fill_char, 10);
        break;

      case 'x':
        arg_num = va_arg(ap, int);
        out_len = add_number(out, out_len, arg_num, 0, width, is_left, fill_char, 16);
        break;

      case 's' :
        arg_s = (char*) va_arg(ap, char*);
        out_len = add_string(out, out_len, arg_s, width, is_left);
        break;
      
      case 'c' :
        agr_c = (char) va_arg(ap, int);
        out_len = add_char(out, out_len, agr_c, width, is_left);
        break;

      case '\0':
        --p;
        break;

      default:
        break;
    }

  }
  out[out_len] = '\0';
  return out_len;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt); // 指向可变参数列表中的第一个参数 
  int length = vsprintf(out, fmt, args);
  va_end(args);
  return length;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

#endif

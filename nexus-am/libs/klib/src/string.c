#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *str) {
  if (str == NULL)
    return 0;
  int len = 0;
  while (str[len]) len++;
  return len;
}

char *strcpy(char* dst,const char* src) {
  int i = 0;
  for ( ; src[i]; i++) {
    dst[i] = src[i];
  }
  dst[i] = 0;
  return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
  int i = 0;
  while (src[i] && i < n) {
    dst[i] = src[i];
    i++;
  }
  while (i < n) {
    dst[i++] = '\0';
  }
  return dst;
}

//
char* strcat(char* dst, const char* src) {
  int i = strlen(dst);
  while (*src) {
    dst[i++] = *src++;
  }
  dst[i] = '\0';
  return dst;
}

//
int strcmp(const char* s1, const char* s2) {
  while (*s1 && *s2 && *s1 == *s2) {
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  int i = 0;
  while (*s1 && *s2 && *s1 == *s2) {
    s1++;
    s2++;
    if (i == n) {
      return 0;
    }
  }
  return *s1 - *s2;
}

void* memset(void* s,int c,size_t n) {
  unsigned char* cs = (unsigned char *)s;
  for (int i = 0; i < n; i++) {
    cs[i] = (unsigned char)c;
  }
  return s;
}

void* memcpy(void* dest, const void* src, size_t n) {
  int i;
  const char *s = src;
  char *d = dest;

  assert(dest && src);
  assert((src + n <= dest) || (dest + n <= src));

  for (i = 0; i < n; i++)
    d[i] = s[i];
  return dest;
}

int memcmp(const void* s1, const void* s2, size_t n){
  char* str1 = (char*)s1;
  char* str2 = (char*)s2;
  for (int i = 0; i < n; i++) {
    if (str1[i] != str2[i]) {
      return str1[i] - str2[i];
    }
  }
  return 0;
}

#endif

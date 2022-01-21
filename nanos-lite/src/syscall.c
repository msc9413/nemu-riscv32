#include "common.h"
#include "syscall.h"
#include "fs.h"
#include "proc.h"

extern void naive_uload(PCB *pcb, const char *filename);

static int program_break;

int sys_yield(){
  _yield();
  return 0;
}

int sys_execve(const char *fname, char * const argv[], char *const envp[]) {
  printf("%s\n", fname);
  naive_uload(NULL, fname);
}

void sys_exit(uintptr_t arg){
   _halt(arg);
  //sys_execve("/bin/init", NULL, NULL);
}

int sys_write(int fd, const void*buf, size_t count){
  // if (fd==1 || fd==2) {
  //   for(int i = 0; i < count; i++) {
  //       _putc(((char*)buf)[i]);
  //   }
  //   return count;
  // }
  // return 0;
  return fs_write(fd, buf, count);
}

int sys_brk(int addr){
  program_break = addr;
  return 0;
}

int sys_open(const char*path, int flags, int mode){
  return fs_open(path, flags, mode);
}

int sys_close(int fd){
  return fs_close(fd);
}

int sys_read(int fd, void*buf, size_t count){
  return fs_read(fd, buf, count);
}

size_t sys_lseek(int fd, size_t offset, int whence){
  return fs_lseek(fd, offset, whence);
}

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
    case SYS_yield:
      c->GPRx = sys_yield();
      break;
    case SYS_exit:
      sys_exit(a[1]);
      break;
    case SYS_write:
      c->GPRx = sys_write(a[1], (void*)(a[2]), a[3]);
      break;
    case SYS_brk:
      c->GPRx = sys_brk(a[1]);
      break;
    case SYS_read:
      c->GPRx = sys_read(a[1], (void*)(a[2]), a[3]);
      break;
    case SYS_lseek:
      c->GPRx = sys_lseek(a[1], a[2], a[3]);
      break;
    case SYS_open:
      c->GPRx = sys_open((const char *)a[1], a[2], a[3]);
      break;
    case SYS_close:
      c->GPRx = sys_close(a[1]);
      break;
    case SYS_execve:
      c->GPRx = sys_execve(a[1], a[2], a[3]);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}

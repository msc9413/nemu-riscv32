#include "fs.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

extern size_t ramdisk_read(void*, size_t, size_t);
extern size_t ramdisk_write(const void*, size_t, size_t);
extern size_t serial_write(const void*, size_t, size_t);
extern size_t events_read(void*, size_t, size_t);
extern size_t fb_write(const void*, size_t, size_t);
extern size_t fbsync_write(const void*, size_t, size_t);
extern size_t dispinfo_read(void*, size_t, size_t);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0, 0, invalid_read, invalid_write},
  {"stdout", 0, 0, 0, invalid_read, serial_write},
  {"stderr", 0, 0, 0, invalid_read, serial_write},
  {"/dev/events", 0xffffff, 0, 0, events_read, invalid_write},
  {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
  {"/dev/fbsync", 0xffff, 0, 0, invalid_read, fbsync_write},
  {"/proc/dispinfo", 128, 0, 0, dispinfo_read, invalid_write},
  {"/dev/tty", 0, 0, 0, invalid_read, serial_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  int fb = fs_open("/dev/fb", 0, 0);
  file_table[fb].size = screen_width() * screen_height() * 4;
}

int fs_open(const char *pathname, int flags, int mode) {
  // Log("open file : %s", pathname);
  // 遍历所有文件
  for (int i = 0; i < NR_FILES; i++) {
    if(strcmp(pathname, file_table[i].name) == 0){
      return i;
    }
  }
  assert(!"The file doesn't exist");
}

size_t fs_read(int fd, void *buf, size_t len) {
  // Log("fd = %d read : %s", fd, (char*)buf);
	assert(fd >= 0 && fd < NR_FILES);

  // 防止越界
	int read_len = len;
  if (file_table[fd].size > 0 && file_table[fd].size - file_table[fd].open_offset < read_len) {
    read_len = file_table[fd].size - file_table[fd].open_offset;
  }
  assert(read_len >= 0);

	size_t ret = 0;
  if (file_table[fd].read) {        // 文件定义了读函数，直接调用
    ret = file_table[fd].read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, read_len);
  } else {
    ret = ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, read_len);
  }

  file_table[fd].open_offset += ret;
	return ret;
}

size_t fs_write(int fd, const void *buf, size_t len){
  // Log("fd = %d write : %s", fd, (char*)buf);
	assert(fd >= 0 && fd < NR_FILES);

  // 防止越界
	int write_len = len;
	if (file_table[fd].size > 0 && file_table[fd].open_offset + len > file_table[fd].size) {
	  write_len = file_table[fd].size - file_table[fd].open_offset;
  }
	assert(write_len >= 0);
	
	size_t ret = 0;
  if (file_table[fd].write) {       // 文件定义了写函数，直接调用
    ret = file_table[fd].write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, write_len);
  } else {
		ret = ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, write_len);
  }

  file_table[fd].open_offset += ret;
	return ret;
}

size_t fs_lseek(int fd, size_t offset, int whence){
	assert(fd >= 0 && fd < NR_FILES);
	switch (whence) {
		case SEEK_SET: 
			file_table[fd].open_offset = offset;
			break;
		case SEEK_CUR:
			file_table[fd].open_offset += offset;
			break;
		case SEEK_END:
			file_table[fd].open_offset = file_table[fd].size + offset;
			break;
		default: panic("The whence is not implementation");
	}
  assert(file_table[fd].open_offset <= file_table[fd].size);
	return file_table[fd].open_offset;
}

int fs_close(int fd){
  file_table[fd].open_offset = 0;
  return 0;
}
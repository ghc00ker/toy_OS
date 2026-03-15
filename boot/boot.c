#include "boot.h"
#include <assert.h>

// DO NOT DEFINE ANY NON-LOCAL VARIBLE!

void load_kernel() {
  // comment us in WEEK-1-os-start.
  // char hello[] = {'\n', 'h', 'e', 'l', 'l', 'o', '\n', 0};
  // putstr(hello);
  // while (1) ;
  // remove both lines above before write codes below
  Elf32_Ehdr *elf = (void *)0x8000; // ELF程序头
  copy_from_disk(elf, 255 * SECTSIZE, SECTSIZE); // 操作系统kernel是装在磁盘的第1~255扇区的一个ELF文件
  Elf32_Phdr *ph, *eph; // ELF程序头表（数据段&代码段）
  ph = (void*)((uint32_t)elf + elf->e_phoff); // 程序头表的位置，elf的位置+eph的偏移量
  eph = ph + elf->e_phnum;   // 指针的加减运算以它所指向的数据类型的大小为单位，eph就是end program header
  for (; ph < eph; ph++) {
    if (ph->p_type == PT_LOAD) {
      // TODO: Week 1, Load kernel and jump
      memcpy((void *)(ph->p_vaddr), (void *)(elf) + (ph->p_offset), ph->p_filesz);
      memset((void *)(ph->p_vaddr) + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
      // TODO();
    }
  }
  uint32_t entry = (elf->e_entry); // change me in WEEK1-os-start
  ((void(*)())entry)();
}

#pragma once
#include "common/common.h"

#define SATP_SV32 (1u << 31)
#define PAGE_V    (1 << 0)   // "Valid" bit (entry is enabled)
#define PAGE_R    (1 << 1)   // Readable
#define PAGE_W    (1 << 2)   // Writable
#define PAGE_X    (1 << 3)   // Executable
#define PAGE_U    (1 << 4)   // User (accessible in user mode)，如果是kernel space 的地址，這一碼應為0


void map_page(uint32_t *table1, uint32_t vaddr, paddr_t paddr, uint32_t flags);
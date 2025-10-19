#pragma once

// 給現有的型別取新名字，讓程式碼更清楚！
typedef int bool;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef uint32_t size_t;
typedef uint32_t paddr_t; //代表 "實體" 記憶體位址的型別
typedef uint32_t vaddr_t; //代表 "虛擬" 記憶體位址的型別，對應標準函式庫中的 uintptr_t

#define true  1
#define false 0
#define NULL  ((void *) 0)

// 將 value 向上取整至最接近的 align 倍數。align 必須是 2 的次方。
#define align_up(value, align)   __builtin_align_up(value, align)

// 檢查 value 是否為 align 的倍數。align 必須是 2 的次方。
#define is_aligned(value, align) __builtin_is_aligned(value, align)

// 回傳結構體中某成員的偏移量（從結構開頭算起的位元組數）。
#define offsetof(type, member)   __builtin_offsetof(type, member)


// 以 __builtin_ 開頭的 → 編譯器內建功能
// #define 新名字 舊名字
#define va_list  __builtin_va_list //定義一個變數來存放參數
#define va_start __builtin_va_start //開始讀取可變參數
#define va_end   __builtin_va_end //取出下一個參數
#define va_arg   __builtin_va_arg //結束，清理資源


void *memset(void *buf, char c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
char *strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);
void printf(const char *fmt, ...);


#define PAGE_SIZE 4096 //定義一個page大小就是4096KB


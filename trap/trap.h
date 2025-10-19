#pragma once //防止重複包含
#include "common/common.h"


// 編譯器會自己根據以下的結構，計算每個變數的偏移量 （相對於傳進來的sp的偏移量）
// 我們必須確保順序跟stack的儲存順序是一致的
// 有了這一段struct，就可以用 trap_frame -> ra 來存取某暫存器
struct trap_frame {
    uint32_t ra; // 第 0 個成員 （offset 0）
    uint32_t gp; // 第 1 個成員 （offset 1）
    uint32_t tp; // 第 2 個成員 （offset 2）
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t sp;
} __attribute__((packed)); //確保沒有填充


// 中斷處理函數，真正的函數在 trap.c 中定義
void kernel_entry(void);   // ← 在 trap.c 中定義


//讀取某csr暫存器
#define READ_CSR(reg)                                                          \
    ({                                                                         \
        unsigned long __tmp;                                                   \
        __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));                  \
        __tmp;                                                                 \
    })

#define WRITE_CSR(reg, value)                                                  \
    do {                                                                       \
        uint32_t __tmp = (value);                                              \
        __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));                \
    } while (0)
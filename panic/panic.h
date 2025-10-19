#pragma once
#include "common/common.h"


struct sbiret {
    long error;
    long value;
};


// 每行結尾都有反斜線
// 會印出： 哪個檔案、哪行 ex. PANIC: kernel.c:102: booted! 

#define PANIC(fmt, ...)                                                        \
    do {                                                                       \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
        while (1) {}                                                           \
    } while (0) // 確保只執行一次



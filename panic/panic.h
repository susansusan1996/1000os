#pragma once
#include "common/common.h"


struct sbiret {
    long error;
    long value;
};


// 每行結尾都有反斜線
// 會印出： 哪個檔案、哪行 ex. PANIC: kernel.c:102: booted! 

// 目前一定要有 while (1) {} 這一行。
// 目的：讓程式卡在這，而不執行sret。因為如果執行sret，就又會回到原本造成 exception 的程式碼，就會造成無窮迴圈
#define PANIC(fmt, ...)                                                        \
    do {                                                                       \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
        while (1) {}          \
    } while (0) // 用do{} while(0) 包起來的原因：讓巨集展開之後，不會因為沒有大括號而少做了一些城市片段





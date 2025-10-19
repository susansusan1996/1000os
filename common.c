#include "common.h"

void putchar(char ch);

void printf(const char *fmt, ...) {
    va_list vargs; // 定義一個變數來存放參數
    va_start(vargs, fmt); // 開始讀取可變參數

    while (*fmt) {
        if (*fmt == '%') {
            fmt++; // Skip '%'
            switch (*fmt) {
                case '\0': // '%' 接在%後面，表示要印%
                    putchar('%');
                    goto end;
                case '%': // Print '%'
                    putchar('%');
                    break;
                case 's': { // Print a NULL-terminated string.
                    const char *s = va_arg(vargs, const char *); //va_arg: 取出下一個參數
                    while (*s) {
                        putchar(*s);
                        s++;
                    }
                    break;
                }
                case 'd': { // Print an integer in decimal.
                    int value = va_arg(vargs, int);
                    unsigned magnitude = value; // https://github.com/nuta/operating-system-in-1000-lines/issues/64
                    if (value < 0) {
                        putchar('-');
                        magnitude = -magnitude;
                    }

                    unsigned divisor = 1;

                    //先找出最大的divisor
                    // 找最高位：用 divisor 找出從哪一位開始
                    while (magnitude / divisor > 9)
                        divisor *= 10;
                    
                    //每除一個位數，就把devisor再除10
                    while (divisor > 0) {
                        putchar('0' + magnitude / divisor); //'0'是因為：要把int變字串ｓ
                        magnitude %= divisor;
                        divisor /= 10;
                    }

                    break;

                }
                case 'x': { // 將 32 bit 轉換成16進位
                    unsigned value = va_arg(vargs, unsigned);

                    // 4個bit一組，共8組
                    for (int i = 7; i >= 0; i--) {
                        unsigned nibble = (value >> (i * 4)) & 0xf; //右移、遮罩
                        putchar("0123456789abcdef"[nibble]);
                    }
                }
            }
        } else {
            putchar(*fmt);
        }

        fmt++;
    }

end:
    va_end(vargs);
}


// ------------------------------------------------------------------------------------
// 將一字串cpy到另一個地址位置
void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *) dst;
    const uint8_t *s = (const uint8_t *) src;
    while (n--)
        *d++ = *s++;
    return dst;
}

// 複製 "ABC" (3 bytes)

// Before:
// src: 0x1000 → ['A']['B']['C']
// dst: 0x2000 → [ ? ][ ? ][ ? ]

// Step 1:
// src: 0x1000 → ['A']['B']['C']
//               讀取 'A' ↓
// dst: 0x2000 → ['A'][ ? ][ ? ]
//               寫入 ↑

// Step 2:
// src: 0x1001 → ['A']['B']['C']
//                    讀取 'B' ↓
// dst: 0x2001 → ['A']['B'][ ? ]
//                    寫入 ↑

// Step 3:
// src: 0x1002 → ['A']['B']['C']
//                         讀取 'C' ↓
// dst: 0x2002 → ['A']['B']['C']
//                         寫入 ↑

// Done! ✅


// !! 注意 ------------------------------------------------------------------------------------
// *p++   // 取值後，指標移動
// (*p)++ // 指標不動，值加 1


// 將某個陣列都填滿同一個值-----------------------------------------------------------------------
void *memset(void *buf, char c, size_t n) {
    // 第 1 步：轉換指標型別
    uint8_t *p = (uint8_t *) buf;
    
    // 第 2 步：逐個填值
    while (n--)
        *p++ = c;
    
    // 第 3 步：回傳緩衝區位址
    return buf;
}


// strcpy 在複製時，即使 src 的長度超過了 dst 的記憶體空間，也會繼續複製。這很容易導致錯誤與安全漏洞，因此實務上強烈建議不要使用 strcpy
char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while (*src) //while (*src != '\0') 
        *d++ = *src++;
    *d = '\0';
    return dst;
}


// 比較字串
// !strcmp(s1, s2) 為 true 時，表示兩個字串相同（即函式回傳 0）
int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2) //如果s1、s2字串不同，就跳出迴圈
            break;
        s1++;
        s2++;
    }

    //兩個字元相減，看是誰比較大，如果兩字串相同，會是 ‘/0’ - ‘/0’ = 0
    return *(unsigned char *)s1 - *(unsigned char *)s2; 
}
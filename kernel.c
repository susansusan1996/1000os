#include "kernel.h"
#include "common/common.h"
#include "trap/trap.h"

extern char __bss[], __bss_end[], __stack_top[];

// 啟動代碼 - 程序的真正入口點
__attribute__((section(".text.boot")))
void boot(void) {
    // 只設置堆疊和跳轉
    __asm__ __volatile__(
        "la sp, __stack_top\n"
        "j kernel_main\n"
    );
}


// `ecall` 執行完後：
// - `a0` 存放**錯誤碼**（0 表示成功）
// - `a1` 存放**回傳值**

// 把這兩個值包裝成結構體回傳。

// ---

// ## 📝 完整流程圖
// ```
// 呼叫 sbi_call(ch, 0, 0, 0, 0, 0, 0, 1)
//     ↓
// 把參數放到暫存器
//     a0 = ch
//     a1 = 0
//     a2 = 0
//     a3 = 0
//     a4 = 0
//     a5 = 0
//     a6 = 0 (fid)
//     a7 = 1 (eid)
//     ↓
// 執行 ecall 指令
//     ↓
// CPU 切換到特權模式
//     ↓
// SBI 固件處理請求
// 「哦，eid=1 是要印字元！」
// 「a0 裡面是字元 ch」
//     ↓
// 印出字元到螢幕
//     ↓
// 回傳結果到 a0, a1
//     ↓
// 包裝成 struct sbiret 回傳

struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4,
                       long arg5, long fid, long eid) {
    register long a0 __asm__("a0") = arg0;  // ch,  arg0: 要印的字元
    register long a1 __asm__("a1") = arg1;  // 0,   arg1: 不需要
    register long a2 __asm__("a2") = arg2;  // 0,   arg2: 不需要
    register long a3 __asm__("a3") = arg3;  // 0,   arg3: 不需要
    register long a4 __asm__("a4") = arg4;  // 0,   arg4: 不需要
    register long a5 __asm__("a5") = arg5;  // 0,   arg5: 不需要
    register long a6 __asm__("a6") = fid;   // fid:  功能 ID = 0（Console Putchar）
    register long a7 __asm__("a7") = eid;   // eid:  擴展 ID = 1（Legacy Console Extension）


    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                           "r"(a6), "r"(a7)
                         : "memory");
    return (struct sbiret){.error = a0, .value = a1};
}

void putchar(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1);
}

// void kernel_main(void) {
//     const char *s = "\n\nHello World!\n";
//     for (int i = 0; s[i] != '\0'; i++) {
//         putchar(s[i]);
//     }

//     for (;;) {
//         __asm__ __volatile__("wfi");
//     }
// }

// void kernel_main(void) {
//     printf("\n\nHello %s\n", "World!");
//     printf("1 + 2 = %d, %x\n", 1 + 2, 0x1234abcd);

//     // 無窮迴圈
//     for (;;) {
//         //WFI = Wait For Interrupt（等待中斷）
//         __asm__ __volatile__("wfi");
//     }
// }


// void kernel_main(void) {
//     memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

//     PANIC("booted!");
//     printf("unreachable here!\n");
// }



void kernel_main(void) {
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    WRITE_CSR(stvec, (uint32_t) kernel_entry); // new
    __asm__ __volatile__("unimp"); // new
}
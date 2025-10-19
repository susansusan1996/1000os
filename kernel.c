#include "panic/panic.h"
#include "common/common.h"
#include "trap/trap.h"
#include "alloc_pages/alloc_pages.h"
#include "process/process.h"
#include "putchar/putchar.h"

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



// putchar-------------------------------------------------------------------------
// void kernel_main(void) {
//     const char *s = "\n\nHello World!\n";
//     for (int i = 0; s[i] != '\0'; i++) {
//         putchar(s[i]);
//     }

//     for (;;) {
//         __asm__ __volatile__("wfi");
//     }
// }


//printf --------------------------------------------------------------------------
// void kernel_main(void) {
//     printf("\n\nHello %s\n", "World!");
//     printf("1 + 2 = %d, %x\n", 1 + 2, 0x1234abcd);

//     // 無窮迴圈
//     for (;;) {
//         //WFI = Wait For Interrupt（等待中斷）
//         __asm__ __volatile__("wfi");
//     }
// }


//PANIC --------------------------------------------------------------------------
// void kernel_main(void) {
//     memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

//     PANIC("booted!");
//     printf("unreachable here!\n");
// }


//trap --------------------------------------------------------------------------
//kernel_entry                    stvec
// （函數本體）                    （CSR 暫存器）

// 0x80200000: ┌─────────────┐    ┌─────────────┐
//             │ csrw sscratch│    │   stvec     │
//             │ addi sp, -124│    │ = 0x80200000│ ← 儲存位址
//             │ sw ra, ...   │    └─────────────┘
//             │ ...          │           │
//             └─────────────┘           │
//                  ↑                    │
//                  └────────────────────┘
//                       指向
// void kernel_main(void) {
//     memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

//     // stvec: 中斷向量暫存器，用來保存，自訂的中斷處理程式的位置
//     // kernel_entry : 我們自己定義的中斷處理程式，轉為 uint32_t 是因為該位置是一個 32 bit 的地址
//     WRITE_CSR(stvec, (uint32_t) kernel_entry); // 設定中斷向量
//     __asm__ __volatile__("unimp"); // 故意執行非法指令，測試是否真的有去執行中斷
//     // unimp 是一個「偽指令」。
//     // 根據 RISC-V 組合語言程式設計手冊，組譯器會將 unimp 轉換為以下指令：
//     // csrrw x0, cycle, x0
//     // 這行指令試圖將 cycle 暫存器的值寫入 x0，同時從 x0 讀出。
//     // 但由於 cycle 是唯讀（read-only）暫存器，CPU 會判定這是無效指令，並觸發非法指令例外。
// }



//alloc_pages --------------------------------------------------------------------------
// void kernel_main(void) {
//     memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

//     paddr_t paddr0 = alloc_pages(2);
//     paddr_t paddr1 = alloc_pages(1);
//     printf("alloc_pages test: paddr0=%x\n", paddr0);
//     printf("alloc_pages test: paddr1=%x\n", paddr1);
//     //以上print出來應該要差8KB（也就是兩個page，因為這邊我們自己定義一個page是4KB( 4096 byte )）

//     PANIC("booted!");
// }

// // PS: 
// // 4KB = 4096 = 0x1000（十六進位表示）。
// // 因此，頁面對齊的位址在十六進位表示下會呈現出整齊的對齊效果。



//process --------------------------------------------------------------------------
void kernel_main(void) {
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    WRITE_CSR(stvec, (uint32_t) kernel_entry);

    proc_a = create_process((uint32_t) proc_a_entry);
    proc_b = create_process((uint32_t) proc_b_entry);
    proc_a_entry();

    PANIC("unreachable here!");
}
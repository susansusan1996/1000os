#include "panic/panic.h"
#include "common/common.h"
#include "trap/trap.h"



//-----------------------------------------------------------------------
//excpetion處理


// 高位址
//     ┌─────────────┐    
//     │   舊的堆疊   │
//     ├─────────────┤ ← 原本的 sp（已存到 sscratch）
//     │ ra  (offset 0)  │ ← 新的 sp 指向這裡！, 
//                           我們只需要對handle_trap傳入新的sp地址，
//                           就能夠取得整塊保留給該function的stack
//     │ gp  (offset 4)  │
//     │ tp  (offset 8)  │
//     │ t0  (offset 12) │
//     │ ...             │
//     │ sp  (offset 120)│
//     └─────────────┘
// 低位址
void handle_trap(struct trap_frame *f) {
    printf("trap_frame: %d", f);
    uint32_t scause = READ_CSR(scause);
    uint32_t stval = READ_CSR(stval);
    uint32_t user_pc = READ_CSR(sepc);

    PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
}



//中斷處理是特殊情況：
//  中斷可能在任何時候發生
//  我們需要保存所有暫存器（不只是部分）
//  需要特殊的返回方式（sret，不是 ret）


// ### 為什麼需要 sscratch？
// ```
// 需求：保存原始 sp
// 問題：所有 32 個通用暫存器都可能有重要的值
// 解決：用 sscratch（第 33 個暫存器，專門用來暫存）
// ```


// --------------------------------沒有重設sscratch （kernel stack 的 sp）， 不安全 --------------------------------------------------------
// __attribute__((naked))     // 裸函數（編譯器什麼都不做）
// __attribute__((aligned(4))) // **意思：** 這個函數的位址必須是 **4 的倍數**
// void kernel_entry(void) {
//     __asm__ __volatile__(
//         "csrw sscratch, sp\n"    //先將舊sp存到sscratch暫存器
//         "addi sp, sp, -4 * 31\n" //sp先跑到預留的stack空間的最下面
//         "sw ra,  4 * 0(sp)\n"    //慢慢從sp往上填滿欲保留的暫存器內容
//         "sw gp,  4 * 1(sp)\n"
//         "sw tp,  4 * 2(sp)\n"
//         "sw t0,  4 * 3(sp)\n"
//         "sw t1,  4 * 4(sp)\n"
//         "sw t2,  4 * 5(sp)\n"
//         "sw t3,  4 * 6(sp)\n"
//         "sw t4,  4 * 7(sp)\n"
//         "sw t5,  4 * 8(sp)\n"
//         "sw t6,  4 * 9(sp)\n"
//         "sw a0,  4 * 10(sp)\n" //保存a0暫存器自己的內容
//         "sw a1,  4 * 11(sp)\n"
//         "sw a2,  4 * 12(sp)\n"
//         "sw a3,  4 * 13(sp)\n"
//         "sw a4,  4 * 14(sp)\n"
//         "sw a5,  4 * 15(sp)\n"
//         "sw a6,  4 * 16(sp)\n"
//         "sw a7,  4 * 17(sp)\n"
//         "sw s0,  4 * 18(sp)\n"
//         "sw s1,  4 * 19(sp)\n"
//         "sw s2,  4 * 20(sp)\n"
//         "sw s3,  4 * 21(sp)\n"
//         "sw s4,  4 * 22(sp)\n"
//         "sw s5,  4 * 23(sp)\n"
//         "sw s6,  4 * 24(sp)\n"
//         "sw s7,  4 * 25(sp)\n"
//         "sw s8,  4 * 26(sp)\n"
//         "sw s9,  4 * 27(sp)\n"
//         "sw s10, 4 * 28(sp)\n"
//         "sw s11, 4 * 29(sp)\n"

//         "csrr a0, sscratch\n" //將舊sp從sscratch暫存器讀出來，放到a0裡，a0現在裝的是舊sp
//         "sw a0, 4 * 30(sp)\n" //將a0裝的舊sp保存到預留的stack空間

//         "mv a0, sp\n" //將新sp move 到 a0 暫存器，供 handle_trap 當參數使用
//         "call handle_trap\n"

//         "lw ra,  4 * 0(sp)\n" // 有問題！！ 執行不到？？？？ 因為上面的 handle_trap 會卡在 panic
//         "lw gp,  4 * 1(sp)\n"
//         "lw tp,  4 * 2(sp)\n"
//         "lw t0,  4 * 3(sp)\n"
//         "lw t1,  4 * 4(sp)\n"
//         "lw t2,  4 * 5(sp)\n"
//         "lw t3,  4 * 6(sp)\n"
//         "lw t4,  4 * 7(sp)\n"
//         "lw t5,  4 * 8(sp)\n"
//         "lw t6,  4 * 9(sp)\n"
//         "lw a0,  4 * 10(sp)\n"
//         "lw a1,  4 * 11(sp)\n"
//         "lw a2,  4 * 12(sp)\n"
//         "lw a3,  4 * 13(sp)\n"
//         "lw a4,  4 * 14(sp)\n"
//         "lw a5,  4 * 15(sp)\n"
//         "lw a6,  4 * 16(sp)\n"
//         "lw a7,  4 * 17(sp)\n"
//         "lw s0,  4 * 18(sp)\n"
//         "lw s1,  4 * 19(sp)\n"
//         "lw s2,  4 * 20(sp)\n"
//         "lw s3,  4 * 21(sp)\n"
//         "lw s4,  4 * 22(sp)\n"
//         "lw s5,  4 * 23(sp)\n"
//         "lw s6,  4 * 24(sp)\n"
//         "lw s7,  4 * 25(sp)\n"
//         "lw s8,  4 * 26(sp)\n"
//         "lw s9,  4 * 27(sp)\n"
//         "lw s10, 4 * 28(sp)\n"
//         "lw s11, 4 * 29(sp)\n"
//         "lw sp,  4 * 30(sp)\n"
//         "sret\n"  //sret: 從監督者模式返回
//     );
// }



// --------------------------------有重設sscratch （kernel stack 的 sp）， 安全 --------------------------------------------------------
__attribute__((naked))     // 裸函數（編譯器什麼都不做）
__attribute__((aligned(4))) // **意思：** 這個函數的位址必須是 **4 的倍數**
void kernel_entry(void) {
    __asm__ __volatile__(     
        "csrrw sp, sscratch, sp\n" //交換sp與sscratch的值，交換後：sp是kernel stack 的 sp <==> sscratch 是原本trap的user space 的 sp
        
        "addi sp, sp, -4 * 31\n" //sp先跑到預留的 kernel stack空間的最下面
        "sw ra,  4 * 0(sp)\n"    //慢慢從sp往上填滿欲保留的暫存器內容
        "sw gp,  4 * 1(sp)\n"
        "sw tp,  4 * 2(sp)\n"
        "sw t0,  4 * 3(sp)\n"
        "sw t1,  4 * 4(sp)\n"
        "sw t2,  4 * 5(sp)\n"
        "sw t3,  4 * 6(sp)\n"
        "sw t4,  4 * 7(sp)\n"
        "sw t5,  4 * 8(sp)\n"
        "sw t6,  4 * 9(sp)\n"
        "sw a0,  4 * 10(sp)\n" //保存a0暫存器自己的內容
        "sw a1,  4 * 11(sp)\n"
        "sw a2,  4 * 12(sp)\n"
        "sw a3,  4 * 13(sp)\n"
        "sw a4,  4 * 14(sp)\n"
        "sw a5,  4 * 15(sp)\n"
        "sw a6,  4 * 16(sp)\n"
        "sw a7,  4 * 17(sp)\n"
        "sw s0,  4 * 18(sp)\n"
        "sw s1,  4 * 19(sp)\n"
        "sw s2,  4 * 20(sp)\n"
        "sw s3,  4 * 21(sp)\n"
        "sw s4,  4 * 22(sp)\n"
        "sw s5,  4 * 23(sp)\n"
        "sw s6,  4 * 24(sp)\n"
        "sw s7,  4 * 25(sp)\n"
        "sw s8,  4 * 26(sp)\n"
        "sw s9,  4 * 27(sp)\n"
        "sw s10, 4 * 28(sp)\n"
        "sw s11, 4 * 29(sp)\n"

        "csrr a0, sscratch\n" //將舊sp從sscratch暫存器讀出來，放到a0裡，a0現在裝的是舊sp
        "sw a0, 4 * 30(sp)\n" //將a0裝的舊sp保存到預留的stack空間

        // 重設sscratch （kernel stack 的 sp） !!!
        // 重要，不然第二次中斷發生，系統會crash
        "addi a0, sp, 4 * 31\n"
        "csrw sscratch, a0\n"

        "mv a0, sp\n" //將新sp move 到 a0 暫存器，供 handle_trap 當參數使用
        "call handle_trap\n"

        "lw ra,  4 * 0(sp)\n" // 有問題！！ 執行不到？？？？ 因為上面的 handle_trap 會卡在 panic
        "lw gp,  4 * 1(sp)\n"
        "lw tp,  4 * 2(sp)\n"
        "lw t0,  4 * 3(sp)\n"
        "lw t1,  4 * 4(sp)\n"
        "lw t2,  4 * 5(sp)\n"
        "lw t3,  4 * 6(sp)\n"
        "lw t4,  4 * 7(sp)\n"
        "lw t5,  4 * 8(sp)\n"
        "lw t6,  4 * 9(sp)\n"
        "lw a0,  4 * 10(sp)\n"
        "lw a1,  4 * 11(sp)\n"
        "lw a2,  4 * 12(sp)\n"
        "lw a3,  4 * 13(sp)\n"
        "lw a4,  4 * 14(sp)\n"
        "lw a5,  4 * 15(sp)\n"
        "lw a6,  4 * 16(sp)\n"
        "lw a7,  4 * 17(sp)\n"
        "lw s0,  4 * 18(sp)\n"
        "lw s1,  4 * 19(sp)\n"
        "lw s2,  4 * 20(sp)\n"
        "lw s3,  4 * 21(sp)\n"
        "lw s4,  4 * 22(sp)\n"
        "lw s5,  4 * 23(sp)\n"
        "lw s6,  4 * 24(sp)\n"
        "lw s7,  4 * 25(sp)\n"
        "lw s8,  4 * 26(sp)\n"
        "lw s9,  4 * 27(sp)\n"
        "lw s10, 4 * 28(sp)\n"
        "lw s11, 4 * 29(sp)\n"
        "lw sp,  4 * 30(sp)\n"
        "sret\n"  //sret: 從監督者模式返回
    );
}









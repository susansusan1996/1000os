#include "kernel.h"
#include "common/common.h"
#include "trap/trap.h"

void handle_trap(struct trap_frame *f) {
    uint32_t scause = READ_CSR(scause);
    uint32_t stval = READ_CSR(stval);
    uint32_t user_pc = READ_CSR(sepc);

    PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
}


//-----------------------------------------------------------------------
//excpetion處理
//中斷處理是特殊情況：
//  中斷可能在任何時候發生
//  我們需要保存所有暫存器（不只是部分）
//  需要特殊的返回方式（sret，不是 ret）

// - `csrw` = CSR Write（寫入控制狀態暫存器）
// - `sscratch` = Supervisor Scratch 暫存器（一個臨時暫存器）
// - `sp` = Stack Pointer（堆疊指標）

// ## 📝 完整流程圖
// ```
// 中斷發生
//     ↓
// 原始狀態：
//   sp = 0x80100000 (使用者堆疊)
//   a0 = 42
//     ↓
// ┌─────────────────────────────────┐
// │ csrw sscratch, sp               │ 備份原始 sp
// │   → sscratch = 0x80100000       │
// ├─────────────────────────────────┤
// │ addi sp, sp, -124               │ 修改 sp
// │   → sp = 0x800FFF84             │
// ├─────────────────────────────────┤
// │ sw a0, 4 * 10(sp)               │ 保存 a0 本身
// │   → [sp+40] = 42                │
// ├─────────────────────────────────┤
// │ ... 保存其他暫存器 ...           │
// ├─────────────────────────────────┤
// │ csrr a0, sscratch  ⭐           │ 讀原始 sp 到 a0
// │   → a0 = 0x80100000             │ (覆蓋掉 42，沒關係，已經保存了)
// ├─────────────────────────────────┤
// │ sw a0, 4 * 30(sp)  ⭐           │ 保存原始 sp
// │   → [sp+120] = 0x80100000       │
// └─────────────────────────────────┘
//     ↓
// 現在堆疊上有：
//   - 位置 40: a0 本身的值 (42)
//   - 位置 120: 原始 sp (0x80100000)


// ### **第 1 步：為什麼需要 sscratch？ (sscratch跟a0暫存器的關係)**
// ```
// 需求：保存原始 sp
// 問題：所有 32 個通用暫存器都可能有重要的值
// 解決：用 sscratch（第 33 個暫存器，專門用來暫存）
// ```

// ---

// ### **第 2 步：為什麼後來又用 a0？**
// ```
// 需求：把 sscratch 的值（原始 sp）存到記憶體
// 問題：RISC-V 不能直接 sw sscratch, memory
// 解決：先 csrr 到 a0，再 sw 到記憶體
//      （這時 a0 已經被保存了，可以安全使用）
__attribute__((naked))     // 裸函數（編譯器什麼都不做）
__attribute__((aligned(4))) // **意思：** 這個函數的位址必須是 **4 的倍數**
void kernel_entry(void) {
    __asm__ __volatile__(
        "csrw sscratch, sp\n"    //先將舊sp存到sscratch暫存器
        "addi sp, sp, -4 * 31\n" //sp先跑到預留的stack空間的最下面
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

        "mv a0, sp\n" //將新sp move 到 a0 暫存器，供 handle_trap 當參數使用
        "call handle_trap\n"

        "lw ra,  4 * 0(sp)\n"
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







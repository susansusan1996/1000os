#include "common/common.h"
#include "trap/trap.h"
#include "alloc_pages/alloc_pages.h"
#include "process.h"
#include "panic/panic.h"
#include "putchar/putchar.h"
#include "page_table/page_table.h"

extern char __kernel_base[];
extern char __free_ram_end[];

__attribute__((naked)) void switch_context(uint32_t *prev_sp, uint32_t *next_sp) {
                                                    // 第1個參數         第2個參數
                                                    //  ↓                 ↓
                                                    // 自動放到 a0       自動放到 a1
    __asm__ __volatile__(
        
        // Save callee-saved registers onto the current process's stack.
        "addi sp, sp, -13 * 4\n" // Allocate stack space for 13 4-byte registers
        "sw ra,  0  * 4(sp)\n"   // Save callee-saved registers only
        "sw s0,  1  * 4(sp)\n"
        "sw s1,  2  * 4(sp)\n"
        "sw s2,  3  * 4(sp)\n"
        "sw s3,  4  * 4(sp)\n"
        "sw s4,  5  * 4(sp)\n"
        "sw s5,  6  * 4(sp)\n"
        "sw s6,  7  * 4(sp)\n"
        "sw s7,  8  * 4(sp)\n"
        "sw s8,  9  * 4(sp)\n"
        "sw s9,  10 * 4(sp)\n"
        "sw s10, 11 * 4(sp)\n"
        "sw s11, 12 * 4(sp)\n"

        // Switch the stack pointer.
        "sw sp, (a0)\n"         // *prev_sp = sp;
        "lw sp, (a1)\n"         // Switch stack pointer (sp) here

        // Restore callee-saved registers from the next process's stack.
        "lw ra,  0  * 4(sp)\n"  // Restore callee-saved registers only
        "lw s0,  1  * 4(sp)\n"
        "lw s1,  2  * 4(sp)\n"
        "lw s2,  3  * 4(sp)\n"
        "lw s3,  4  * 4(sp)\n"
        "lw s4,  5  * 4(sp)\n"
        "lw s5,  6  * 4(sp)\n"
        "lw s6,  7  * 4(sp)\n"
        "lw s7,  8  * 4(sp)\n"
        "lw s8,  9  * 4(sp)\n"
        "lw s9,  10 * 4(sp)\n"
        "lw s10, 11 * 4(sp)\n"
        "lw s11, 12 * 4(sp)\n"
        "addi sp, sp, 13 * 4\n"  // We've popped 13 4-byte registers from the stack
        "ret\n"
    );
}



struct process procs[PROCS_MAX]; // All process control structures.

struct process *create_process(uint32_t pc) {
    // Find an unused process control structure.
    struct process *proc = NULL;
    int i;
    for (i = 0; i < PROCS_MAX; i++) {
        if (procs[i].state == PROC_UNUSED) {
            proc = &procs[i];
            break;
        }
    }

    if (!proc)
        PANIC("no free process slots");

    // Stack callee-saved registers. These register values will be restored in
    // the first context switch in switch_context.
    uint32_t *sp = (uint32_t *) &proc->stack[sizeof(proc->stack)];
    *--sp = 0;                      // s11 把sp這個位置再減四，然後將這個位置的值改成0
    *--sp = 0;                      // s10
    *--sp = 0;                      // s9
    *--sp = 0;                      // s8
    *--sp = 0;                      // s7
    *--sp = 0;                      // s6
    *--sp = 0;                      // s5
    *--sp = 0;                      // s4
    *--sp = 0;                      // s3
    *--sp = 0;                      // s2
    *--sp = 0;                      // s1
    *--sp = 0;                      // s0
    *--sp = (uint32_t) pc;          // ra

    // Map kernel pages.
    // 創建process的時候，會同時創立page table,並且拿到該page table的起始位置，
    // 就開始在這個位置上，映射kernel address
    uint32_t *page_table = (uint32_t *) alloc_pages(1); //拿到該page table的起始位置
    for (paddr_t paddr = (paddr_t) __kernel_base;
        paddr < (paddr_t) __free_ram_end; paddr += PAGE_SIZE)
        map_page(page_table, paddr, paddr, PAGE_R | PAGE_W | PAGE_X);

    // Initialize fields.
    proc->pid = i + 1;
    proc->state = PROC_RUNNABLE;
    proc->sp = (uint32_t) sp;
    proc->page_table = page_table;
    return proc;
}


void delay(void) {
    for (int i = 0; i < 30000000; i++)
        __asm__ __volatile__("nop"); // do nothing
}

struct process *proc_a;
struct process *proc_b;


struct process *current_proc; // Currently running process
struct process *idle_proc;    // Idle process


//-- yield ----------------------------- Round-robin (輪流) ----------------------------------------------
// void yield(void) {
//     // Search for a runnable process
//     struct process *next = idle_proc;
//     for (int i = 0; i < PROCS_MAX; i++) {

//         //找下一個process
//         struct process *proc = &procs[(current_proc->pid + i) % PROCS_MAX];
//         if (proc->state == PROC_RUNNABLE && proc->pid > 0) {
//             next = proc;
//             break;
//         }
//     }

//     // If there's no runnable process other than the current one, return and continue processing
//     if (next == current_proc)
//         return;

//     // Context switch
//     struct process *prev = current_proc;
//     current_proc = next;
//     switch_context(&prev->sp, &next->sp);
// }


//-- yield ----------------------------- 固定切換到 idle ----------------------------------------------
// void yield(void) {
//     struct process *next = idle_proc;

//     //把下一個process的最高stack存到sscratch
//     __asm__ __volatile__(
//         "csrw sscratch, %[sscratch]\n"
//         :
//         : [sscratch] "r" ((uint32_t) &next->stack[sizeof(next->stack)])
//     );

//     // Context switch
//     struct process *prev = current_proc;
//     current_proc = next;
//     switch_context(&prev->sp, &next->sp);
// }

// | 部分                                                             | 意思                                                                                                                                |
// | -------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
// | `__asm__`                                                      | 告訴編譯器「這裡要插入組合語言」。                                                                                                                 |
// | `__volatile__`                                                 | 告訴編譯器「不要優化或刪掉這段指令」，一定要照著執行。                                                                                                       |
// | `"csrw sscratch, %[sscratch]\n"`                               | 實際的組合語言指令。`csrw` 是 RISC-V 的「寫入控制暫存器」指令。它會把右邊的值寫進暫存器 `sscratch`。                                                                   |
// | `:`（第一個空欄）                                                     | 這裡是「輸出參數」，但我們沒有輸出，所以空白。                                                                                                           |
// | `:`（第二個欄位）                                                     | 這裡是「輸入參數」。                                                                                                                        |
// | `[sscratch] "r" ((uint32_t)&next->stack[sizeof(next->stack)])` | 表示我們要給組合語言一個輸入變數，名稱叫 `sscratch`。 `"r"` 表示放到一個暫存器（register）。右邊的值是：`(uint32_t)&next->stack[sizeof(next->stack)]`。這就是「下一個行程的堆疊頂端位址」。 |



//-- yield --------- （context switching）時切換該行程（process）的分頁表（page table） ----------------------------------------------
void yield(void) {
    // Search for a runnable process
    struct process *next = idle_proc;
    for (int i = 0; i < PROCS_MAX; i++) {
        //找下一個process
        struct process *proc = &procs[(current_proc->pid + i) % PROCS_MAX];
        if (proc->state == PROC_RUNNABLE && proc->pid > 0) {
            next = proc;
            break;
        }
    }

    // If there's no runnable process other than the current one, return and continue processing
    if (next == current_proc)
        return;

    __asm__ __volatile__(
        "sfence.vma\n" //🚧 清空 TLB
        "csrw satp, %[satp]\n" //🔄 切換到新處程的頁表
        "sfence.vma\n" //🚧 再次清空 TLB (使新頁表生效)
        "csrw sscratch, %[sscratch]\n" //📍 設定新處程的堆疊指標
        :
        // Don't forget the trailing comma!
        : [satp] "r" (SATP_SV32 | ((uint32_t) next->page_table / PAGE_SIZE)),
          [sscratch] "r" ((uint32_t) &next->stack[sizeof(next->stack)])
    );

    // Context switch
    struct process *prev = current_proc;
    current_proc = next;
    switch_context(&prev->sp, &next->sp);
}



void proc_a_entry(void) {
    printf("starting process A\n");
    while (1) {
        putchar('A');
        yield();
        // switch_context(&proc_a->sp, &proc_b->sp);
        delay();
    }
}

void proc_b_entry(void) {
    printf("starting process B\n");
    while (1) {
        putchar('B');
        yield();
        // switch_context(&proc_b->sp, &proc_a->sp);
        delay();
    }
}


#include "common/common.h"
#include "trap/trap.h"
#include "alloc_pages/alloc_pages.h"
#include "process/process.h"
#include "panic/panic.h"
#include "putchar/putchar.h"
#include "page_table.h"


// map_page功能：建立虛擬地址到物理地址的映射關係。
void map_page(uint32_t *table1, uint32_t vaddr, paddr_t paddr, uint32_t flags) {
    if (!is_aligned(vaddr, PAGE_SIZE))
        PANIC("unaligned vaddr %x", vaddr);

    if (!is_aligned(paddr, PAGE_SIZE))
        PANIC("unaligned paddr %x", paddr);

    uint32_t vpn1 = (vaddr >> 22) & 0x3ff;
    if ((table1[vpn1] & PAGE_V) == 0) { //檢查table1[vpn1]，最右邊的valid bit是否為0
        // Create the 1st level page table if it doesn't exist.
        uint32_t pt_paddr = alloc_pages(1);

        //  paddr 除以 PAGE_SIZE，
        // 因為頁表項中應該儲存的是「實體頁框號（physical page number, PPN）」，而不是完整的實體位址。
        table1[vpn1] = ((pt_paddr / PAGE_SIZE) << 10) | PAGE_V;
    }

    // Set the 2nd level page table entry to map the physical page.
    uint32_t vpn0 = (vaddr >> 12) & 0x3ff;

    //用 VPN1 查 table1，取得第二層地址的值
    uint32_t *table0 = (uint32_t *) ((table1[vpn1] >> 10) * PAGE_SIZE);

    //用 VPN0 查 table0，取得實體位置的值
    table0[vpn0] = ((paddr / PAGE_SIZE) << 10) | flags | PAGE_V;
}
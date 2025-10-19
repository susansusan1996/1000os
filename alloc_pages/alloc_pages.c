#include "panic/panic.h"
#include "common/common.h"
#include "trap/trap.h"
#include "alloc_pages.h"


// | 名稱              | 來源          | 型態                     | 用途                              
// | ---------------- | ------------- | ----------------------- | ---------------------------------           
// | `__free_ram`     | linker script | `char[]` (其實是一個地址) | RAM 可分配區起始位址                 
// | `__free_ram_end` | linker script | `char[]` (地址)          | RAM 可分配區結束位址                
// | `extern`         | C 語法關鍵字    | 宣告（非定義）            | 告訴編譯器：這兩個符號在別的檔案裡定義 

extern char __free_ram[], __free_ram_end[];


// 以下的 alloc_pages 函式會動態分配 n 頁的記憶體，並回傳起始位址：
paddr_t alloc_pages(uint32_t n) {
    //注意！！：這裏把next_paddr設為static，所以只有在這個function第一次被呼叫到的時候，才會執行這句話
    static paddr_t next_paddr = (paddr_t) __free_ram;  

    paddr_t paddr = next_paddr;

    next_paddr += n * PAGE_SIZE; //PAGE_SIZE已定義在common.h檔

    //判斷有沒有超出ram的可用範圍
    if (next_paddr > (paddr_t) __free_ram_end)
        PANIC("out of memory");

    // 將這些位置先填充值為0
    memset((void *) paddr, 0, n * PAGE_SIZE);
    return paddr;
}


// vaddr_t  V.S.  paddr_t --------------------------------------------------------------------
// | 類型                    | 英文名稱                    | 代表什麼            | 說明          |
// | ---------------------- | -------------------------- | -------------      | ----------    |
// | vaddr_t ： 虛擬位址      | *virtual address (vaddr)*  | 程式在 CPU 見到的位址 | 經過 MMU 映射  |
// | paddr_t ： 實體位址      | *physical address (paddr)* | 記憶體晶片上的真實位址 | 不經過 MMU 轉換 |

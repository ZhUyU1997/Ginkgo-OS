#include <types.h>
#include <riscv.h>
#include <memlayout.h>
#include <kalloc.h>
#include <vm.h>
#include <task.h>
#include <log.h>
#include <trap.h>
#include <core/class.h>
#include <plic.h>
#include <uart.h>
#include <malloc.h>
#include <core/device.h>
#include <hmap.h>
#include <vfs/vfs.h>
#include <ctype.h>
#include <csr.h>
#include <core/syscall.h>
#include <core/test.h>

static void map_exec_file(const char *file, virtual_addr_t addr)
{
    vfs_mount("virtio-block", "/", "cpio", MOUNT_RO);
    int fd = vfs_open(file, O_RDONLY, 0);
    struct vfs_stat_t st;
    vfs_fstat(fd, &st);

    void *buf = alloc_page(1);
    LOGI("size:" $((int)st.st_size));
    LOGI("buf:" $(buf));

    vfs_read(fd, buf, st.st_size);
    kvmmap(current->pagetable, addr, buf, PAGE_SIZE, PTE_R | PTE_W | PTE_X | PTE_U);
    LOGI();
}

static void do_init()
{
    virtual_addr_t user_addr = 0x20000000UL;
    map_exec_file("/test", user_addr);

    struct pt_regs *regs = current->thread_info.kernel_sp - sizeof(struct pt_regs);
    current->ustack = (virtual_addr_t)alloc_page(1);
    regs->sp = current->ustack + PGSIZE;

    regs->sepc = user_addr;
    regs->sstatus = csr_read(sstatus);
    regs->sstatus &= ~(SSTATUS_SPP);
    regs->sstatus |= SSTATUS_SPIE;
    kvmmap(current->pagetable, current->ustack, current->ustack, PGSIZE, PTE_R | PTE_W | PTE_U);

    local_flush_tlb_all();

    asm volatile("mv tp, %0\n\t"
                 "mv sp, %1\n\t"
                 "la ra, ret_from_exception\n\t"
                 "ret\n\t"
                 :
                 : "r"(current), "r"(regs));
}

void main()
{
    LOGI("main");
    do_init_trap();
    do_init_mem();
    do_init_class();

    kvminit();
    kvminithart();
    task_init();

    do_init_device();
    do_init_vfs();

#ifdef UNIT_TEST
    task_resume(task_create("test", do_all_test));
#else
    task_resume(task_create("init", do_init));
#endif


    local_irq_enable();
    while (1)
        schedule();
}
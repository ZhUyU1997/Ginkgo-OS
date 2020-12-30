
int main()
{
    size_t size = 1024 * 1024 * 128;
    buddy_system_t* system = buddy_create(malloc(size), size);
    mem_pool_control_t control;
    mem_pool_create(&control, system);
#if 0
    mem_pool_alloc(&control, 0);
    mem_pool_alloc(&control, 1);
    mem_pool_alloc(&control, 2);
    mem_pool_alloc(&control, 4);
    mem_pool_alloc(&control, 8);
    mem_pool_alloc(&control, 16);
    mem_pool_alloc(&control, 16 + 1);
    mem_pool_alloc(&control, 16 + 15);
    mem_pool_alloc(&control, 32);
    mem_pool_alloc(&control, 33);
    mem_pool_alloc(&control, 48);
    mem_pool_alloc(&control, 49);
    mem_pool_alloc(&control, 1240);
    mem_pool_alloc(&control, MEM_POOL_MAX_SIZE + 1);

#elif 1
    for (int i = 0; i < 10000000; i++)
    {
        // printv("#" $(i) "\n");
        size_t count1 = rand() % MEM_POOL_MAX_SIZE + 1;
        size_t count2 = rand() % MEM_POOL_MAX_SIZE + 1;
        size_t count3 = rand() % MEM_POOL_MAX_SIZE + 1;

        void* mem1 = mem_pool_alloc(&control, count1);
        void* mem2 = mem_pool_alloc(&control, count2);
        void* mem3 = mem_pool_alloc(&control, count3);

        memset(mem1, 0xff, count1);
        memset(mem2, 0xff, count2);
        memset(mem3, 0xff, count3);

        for (int i = 0; i < count1; i++)
        {
            assert(((char*)mem1)[i] != 0);
            ((char*)mem1)[i] = 0;
        }

        for (int i = 0; i < count2; i++)
        {
            assert(((char*)mem2)[i] != 0);
            ((char*)mem2)[i] = 0;
        }
        for (int i = 0; i < count3; i++)
        {
            assert(((char*)mem3)[i] != 0);
            ((char*)mem3)[i] = 0;
        }

        mem_pool_free(&control, mem1);
        mem_pool_free(&control, mem2);
        mem_pool_free(&control, mem3);
    }
    printv("PASS\n");
#elif 1
    for (int i = 0; i < 10000000; i++)
    {
        // printv("#" $(i) "\n");
        size_t count1 = rand() % 100 + 1;
        size_t count2 = rand() % 100 + 1;
        size_t count3 = rand() % 100 + 1;

        void* mem1 = buddy_alloc_page(system, count1);
        void* mem2 = buddy_alloc_page(system, count2);
        void* mem3 = buddy_alloc_page(system, count3);

        // memset(mem1, 0xff, count1 * PAGE_SIZE);
        // memset(mem2, 0xff, count2 * PAGE_SIZE);
        // memset(mem3, 0xff, count3 * PAGE_SIZE);

        buddy_free_page(system, mem1);
        buddy_free_page(system, mem2);
        buddy_free_page(system, mem3);
    }
    printv("PASS\n");
#else
    for (int i = 0; i < 1000000; i++)
    {
        // printv("#" $(i) "\n");
        size_t count1 = rand() % 20 + 1;
        size_t count2 = rand() % 20 + 1;
        size_t count3 = rand() % 20 + 1;

        void* mem1 = malloc(PAGE_SIZE * count1);
        void* mem2 = malloc(PAGE_SIZE * count2);
        void* mem3 = malloc(PAGE_SIZE * count3);

        // memset(mem1, 0xff, count1 * PAGE_SIZE);
        // memset(mem2, 0xff, count2 * PAGE_SIZE);
        // memset(mem3, 0xff, count3 * PAGE_SIZE);

        free(mem1);
        free(mem2);
        free(mem3);
    }
    printv("PASS\n");
#endif
    return 0;
}
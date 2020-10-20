#define NCPU 1 // maximum number of CPUs

// entry.S needs one stack per CPU.
__attribute__((aligned(16))) char stack0[4096 * NCPU];

typedef unsigned char			u8_t;
typedef unsigned long long		virtual_addr_t;

static inline u8_t read8(virtual_addr_t addr)
{
	return( *((volatile u8_t *)(addr)) );
}

static inline void write8(virtual_addr_t addr, u8_t value)
{
	*((volatile u8_t *)(addr)) = value;
}

volatile virtual_addr_t virt = 0x10000000;

void print(const char *buf, int count)
{
    for (int i = 0; i < count; i++)
    {
        while ((read8(virt + 0x05) & (0x1 << 6)) == 0)
            ;
        write8(virt + 0x00, buf[i]);
    }
}

void start()
{
    print("Hello RISC-V!", sizeof("Hello RISC-V!"));
    while (1)
        ;
}
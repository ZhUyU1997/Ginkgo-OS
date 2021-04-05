#pragma once

#include <typecheck.h>

#define local_irq_enable()	do { raw_local_irq_enable(); } while (0)
#define local_irq_disable()	do { raw_local_irq_disable(); } while (0)

#define local_irq_save(flags)					\
	do {							\
		raw_local_irq_save(flags);			\
	} while (0)
#define local_irq_restore(flags) do { raw_local_irq_restore(flags); } while (0)

#define local_save_flags(flags)	raw_local_save_flags(flags)


#define raw_local_irq_disable()		arch_local_irq_disable()
#define raw_local_irq_enable()		arch_local_irq_enable()
#define raw_local_irq_save(flags)			\
	do {						\
		typecheck(irq_flags_t, flags);	\
		flags = arch_local_irq_save();		\
	} while (0)
#define raw_local_irq_restore(flags)			\
	do {						\
		typecheck(irq_flags_t, flags);	\
		arch_local_irq_restore(flags);		\
	} while (0)
#define raw_local_save_flags(flags)			\
	do {						\
		typecheck(irq_flags_t, flags);	\
		flags = arch_local_save_flags();	\
	} while (0)
#define raw_irqs_disabled_flags(flags)			\
	({						\
		typecheck(irq_flags_t, flags);	\
		arch_irqs_disabled_flags(flags);	\
	})

#define local_irq_enable() \
   do { raw_local_irq_enable(); } while (0)
#define local_irq_disable() \
   do { raw_local_irq_disable(); } while (0)
#define local_irq_save(flags)				\
	do {						\
		raw_local_irq_save(flags);		\
	} while (0)
#define local_irq_restore(flags)			\
	do {						\
		raw_local_irq_restore(flags);	\
	} while (0)


#include <csr.h>

/* read interrupt enabled status */
static inline unsigned long arch_local_save_flags(void)
{
	return csr_read(CSR_SSTATUS);
}

/* unconditionally enable interrupts */
static inline void arch_local_irq_enable(void)
{
	csr_set(CSR_SSTATUS, SR_SIE);
}

/* unconditionally disable interrupts */
static inline void arch_local_irq_disable(void)
{
	csr_clear(CSR_SSTATUS, SR_SIE);
}

/* get status and disable interrupts */
static inline unsigned long arch_local_irq_save(void)
{
	return csr_read_clear(CSR_SSTATUS, SR_SIE);
}

/* test flags */
static inline int arch_irqs_disabled_flags(unsigned long flags)
{
	return !(flags & SR_SIE);
}

/* test hardware interrupt enable bit */
static inline int arch_irqs_disabled(void)
{
	return arch_irqs_disabled_flags(arch_local_save_flags());
}

/* set interrupt enabled status */
static inline void arch_local_irq_restore(unsigned long flags)
{
	csr_set(CSR_SSTATUS, flags & SR_SIE);
}

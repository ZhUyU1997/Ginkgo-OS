

#include <list.h>
#include <types.h>
#include <interrupt/interrupt.h>
#include <core/device.h>
#include <log.h>
#include <malloc.h>
#include <task.h>

class_impl(irqchip_t, device_t){};

static LIST_HEAD(IRQCHIP);

void register_irqchip(irqchip_t *chip)
{
	list_add_tail(&chip->head, &IRQCHIP);
}

static void null_interrupt_function(void *data)
{
}

static irqchip_t *search_irqchip(int irq)
{
	irqchip_t *chip, *n;

	list_for_each_entry_safe(chip, n, &IRQCHIP, head)
	{
		if ((irq >= chip->base) && (irq < (chip->base + chip->nirq)))
			return chip;
	}
	return NULL;
}

bool_t irq_is_valid(int irq)
{
	return search_irqchip(irq) ? TRUE : FALSE;
}

void enable_irq(int irq)
{
	irqchip_t *chip = search_irqchip(irq);

	if (chip && chip->enable)
		chip->enable(chip, irq - chip->base);
}

void disable_irq(int irq)
{
	irqchip_t *chip = search_irqchip(irq);

	if (chip && chip->disable)
		chip->disable(chip, irq - chip->base);
}

void interrupt_handle_exception()
{
	irqchip_t *chip, *n;

	list_for_each_entry_safe(chip, n, &IRQCHIP, head)
	{
		if (chip->dispatch)
			chip->dispatch(chip);
	}
}

struct irq_desc_t *create_interrupt_desc(int nirq)
{
	struct irq_desc_t *desc = (struct irq_desc_t *)calloc(1, sizeof(struct irq_desc_t) * nirq);
	for (int i = 0; i < nirq; i++)
	{
		desc[i].func = null_interrupt_function;
	}
	return desc;
}

bool_t request_irq(int irq, irq_handler_t func, void *data)
{
	struct irqchip_t *chip;
	int offset;

	if (!func)
		return FALSE;

	chip = search_irqchip(irq);
	if (!chip)
		return FALSE;
	offset = irq - chip->base;
	if (chip->desc[offset].func != null_interrupt_function)
		return FALSE;
	chip->desc[offset].func = func;
	chip->desc[offset].data = data;

	if (chip->enable)
		chip->enable(chip, offset);

	return TRUE;
}

bool_t free_irq(int irq)
{
	struct irqchip_t *chip;
	int offset;

	chip = search_irqchip(irq);
	if (!chip)
		return FALSE;

	offset = irq - chip->base;
	if (chip->desc[offset].func == null_interrupt_function)
		return FALSE;

	chip->desc[offset].func = null_interrupt_function;
	chip->desc[offset].data = NULL;

	if (chip->disable)
		chip->disable(chip, offset);

	return TRUE;
}
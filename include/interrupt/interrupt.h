#pragma once

#include <list.h>
#include <types.h>
#include <core/device.h>
#include <core/class.h>

typedef void (*irq_handler_t)(void *data);

struct irq_desc_t
{
    irq_handler_t func;
    irq_handler_t thread_func;
    void *data;
};

class(irqchip_t, device_t)
{
    int base;
    int nirq;
    struct list_head head;
    struct irq_desc_t *desc;
    void (*enable)(struct irqchip_t * chip, int offset);
    void (*disable)(struct irqchip_t * chip, int offset);
    void (*dispatch)(struct irqchip_t * chip);
};

bool_t irq_is_valid(int irq);
bool_t request_irq(int irq, irq_handler_t func, void *data);
bool_t free_irq(int irq);
void enable_irq(int irq);
void disable_irq(int irq);
void interrupt_handle_exception();
void register_irqchip(irqchip_t *chip);
struct irq_desc_t *create_interrupt_desc(int nirq);

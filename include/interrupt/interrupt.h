#pragma once

#include <list.h>
#include <types.h>
#include <core/device.h>
#include <core/class.h>

struct irq_handler_t
{
    void (*func)(void *data);
    void *data;
};

class(irqchip_t, device_t)
{
    int base;
    int nirq;
    struct list_head head;
    struct irq_handler_t *handler;
    void (*enable)(struct irqchip_t * chip, int offset);
    void (*disable)(struct irqchip_t * chip, int offset);
    void (*dispatch)(struct irqchip_t * chip);
};

bool_t irq_is_valid(int irq);
bool_t request_irq(int irq, void (*func)(void *), void *data);
bool_t free_irq(int irq);
void enable_irq(int irq);
void disable_irq(int irq);
void interrupt_handle_exception();
void register_irqchip(irqchip_t *chip);

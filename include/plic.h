#pragma once

void plicinit(void);
void plicinithart(void);
int plic_claim(void);
void plic_complete(int irq);
void plic_handle_interrupt();
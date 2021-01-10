#pragma once

void uart_init(void);
void uart_putc(int c);
int uart_getc(void);

void uart_interrupt();
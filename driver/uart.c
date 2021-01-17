//
// low-level driver routines for 16550a UART.
//

#include <types.h>
#include <memlayout.h>
#include <riscv.h>
#include <io.h>
#include <log.h>

// the UART control registers are memory-mapped
// at address UART0. this macro returns the
// address of one of the registers.
#define Reg(reg) ((volatile unsigned char *)(UART0 + reg))

// the UART control registers.
// some have different meanings for
// read vs write.
// see http://byterunner.com/16550.html
#define RHR 0                  // receive holding register (for input bytes)
#define THR 0                  // transmit holding register (for output bytes)
#define IER 1                  // interrupt enable register
#define IER_RX_ENABLE (1 << 0) // receiver ready interrupt.
#define IER_TX_ENABLE (1 << 1) // transmitter empty interrupt.
#define FCR 2                  // FIFO control register
#define FCR_FIFO_ENABLE (1 << 0)
#define FCR_FIFO_CLEAR (3 << 1) // clear the content of the two FIFOs
#define ISR 2                   // interrupt status register
#define LCR 3                   // line control register
#define LCR_EIGHT_BITS (3 << 0)
#define LCR_BAUD_LATCH (1 << 7) // special mode to set baud rate
#define LSR 5                   // line status register
#define LSR_RX_READY (1 << 0)   // input is waiting to be read from RHR
#define LSR_TX_IDLE (1 << 5)    // THR can accept another character to send

void uart_init(void)
{
    // disable interrupts.
    write8(UART0 + IER, 0x00);
    // special mode to set baud rate.
    write8(UART0 + LCR, LCR_BAUD_LATCH);
    // LSB for baud rate of 38.4K.
    write8(UART0 + 0, 0x03);
    // MSB for baud rate of 38.4K.
    write8(UART0 + 1, 0x00);
    // leave set-baud mode,
    // and set word length to 8 bits, no parity.
    write8(UART0 + LCR, LCR_EIGHT_BITS);
    // reset and enable FIFOs.
    write8(UART0 + FCR, FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);
    // enable transmit and receive interrupts.
    write8(UART0 + IER, IER_TX_ENABLE | IER_RX_ENABLE);
    write8(UART0 + IER, IER_RX_ENABLE);
}

// if the UART is idle, and a character is waiting
// in the transmit buffer, send it.
void uart_putc(int c)
{
    if ((read8(UART0 + LSR) & LSR_TX_IDLE) == 0)
    {
        // the UART transmit holding register is full,
        return;
    }

    write8(UART0 + THR, c);
}

int uart_getc(void)
{
    if (read8(UART0 + LSR) & 0x01)
    {
        // input data is ready.
        return read8(UART0 + RHR);
    }
    else
    {
        return -1;
    }
}

void uart_interrupt(void)
{
    while (1)
    {
        int c = uart_getc();
        if (c == -1)
            break;
    }
}

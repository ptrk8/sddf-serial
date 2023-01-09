#include "imx_uart.h"


bool imx_uart_init(
        imx_uart_t *imx_uart,
        uintptr_t base_vaddr,
        bool auto_insert_carriage_return
) {
    if (imx_uart == NULL) {
        return false;
    }
    /* Save pointer to the registers. */
    imx_uart->regs = imx_uart_regs_get(base_vaddr);
    /* Save user's auto carriage return preference. */
    imx_uart->auto_insert_carriage_return = auto_insert_carriage_return;

//    /* Software reset */
//    imx_uart->regs->cr2 &= ~UART_CR2_SRST;
//    while (!(imx_uart->regs->cr2 & UART_CR2_SRST));
//
    /* Sets the Line Protocol for the Serial Device to 8N1 (8 bits, no parity,
     * one stop bit), which is a common default. */
    int ret_line_protocol = imx_uart_regs_set_line_protocol(
            imx_uart->regs,
            115200,
            8,
            PARITY_NONE,
            1
    );
    if (ret_line_protocol < 0) {
        return false;
    }

     /* Enable the UART */
    imx_uart->regs->cr1 |= UART_CR1_UARTEN;                /* Enable The uart.                  */
    imx_uart->regs->cr2 |= UART_CR2_RXEN | UART_CR2_TXEN;  /* RX/TX enable                      */
    imx_uart->regs->cr2 |= UART_CR2_IRTS;                  /* Ignore RTS                        */
    imx_uart->regs->cr3 |= UART_CR3_RXDMUXDEL;             /* Configure the RX MUX              */
    /* Initialise the receiver interrupt.                                             */
    imx_uart->regs->cr1 &= ~UART_CR1_RRDYEN;               /* Disable recv interrupt.           */
    imx_uart->regs->fcr &= ~UART_FCR_RXTL_MASK;            /* Clear the rx trigger level value. */
    imx_uart->regs->fcr |= UART_FCR_RXTL(1);               /* Set the rx tigger level to 1.     */
    imx_uart->regs->cr1 |= UART_CR1_RRDYEN;                /* Enable recv interrupt.            */

    /* Return true for successful initialisation. */
    return true;
}


int imx_uart_put_char(
        imx_uart_t *imx_uart,
        int c
) {
    imx_uart_regs_t *regs = imx_uart->regs;

    if (imx_uart_regs_is_tx_fifo_busy(regs)) {
        return -1;
    }
    /* If `auto_insert_carriage_return` is enabled, we first set the `\r`
     * character and then set the `\n` character. */
    if (c == '\n' && imx_uart->auto_insert_carriage_return) {
//    if (c == '\n' && (d->flags & SERIAL_AUTO_CR)) { /* Original implementation. */
        /* write CR first */
        regs->txd = '\r';
        /* if we transform a '\n' (LF) into '\r\n' (CR+LF) this shall become an
         * atom, ie we don't want CR to be sent and then fail at sending LF
         * because the TX FIFO is full. Basically there are two options:
         *   - check if the FIFO can hold CR+LF and either send both or none
         *   - send CR, then block until the FIFO has space and send LF.
         * Assuming that if SERIAL_AUTO_CR is set, it's likely this is a serial
         * console for logging, so blocking seems acceptable in this special
         * case. The IMX6's TX FIFO size is 32 byte and TXFIFO_EMPTY is cleared
         * automatically as soon as data is written from regs->txd into the
         * FIFO. Thus the worst case blocking is roughly the time it takes to
         * send 1 byte to have room in the FIFO again. At 115200 baud with 8N1
         * this takes 10 bit-times, which is 10/115200 = 86,8 usec.
         */
        while (imx_uart_regs_is_tx_fifo_busy(regs)) {
            /* busy loop */
        }
    }

    regs->txd = c;
    return c;
}


int imx_uart_get_char(
        imx_uart_t *imx_uart
) {
    imx_uart_regs_t *regs = imx_uart->regs;
    uint32_t reg = 0;
    int c = -1;

    if (regs->sr2 & UART_SR2_RXFIFO_RDR) {
        reg = regs->rxd;
        if (reg & UART_URXD_READY_MASK) {
            c = reg & UART_BYTE_MASK;
        }
    }
    return c;
}




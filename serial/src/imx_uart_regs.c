#include "imx_uart_regs.h"

/**
 * Sets the baud rate.
 * @param regs
 * @param bps
 */
static void imx_uart_regs_set_baud(
        imx_uart_regs_t *regs,
        long bps
);

imx_uart_regs_t *imx_uart_regs_get(uintptr_t base_vaddr) {
    return (imx_uart_regs_t *) base_vaddr;
}

static void imx_uart_regs_set_baud(
        imx_uart_regs_t *regs,
        long bps
) {
    uint32_t bmr, bir, fcr;
    fcr = regs->fcr;
    fcr &= ~UART_FCR_RFDIV_MASK;
    fcr |= UART_FCR_RFDIV(4);
    bir = 0xf;
    bmr = UART_REF_CLK / bps - 1;
    regs->bir = bir;
    regs->bmr = bmr;
    regs->fcr = fcr;
}

int imx_uart_regs_set_line_protocol(
        imx_uart_regs_t *regs,
        long bps,
        int char_size,
        enum serial_parity parity,
        int stop_bits
) {
    uint32_t cr2;
    /* Character size */
    cr2 = regs->cr2;
    if (char_size == 8) {
        cr2 |= UART_CR2_WS;
    } else if (char_size == 7) {
        cr2 &= ~UART_CR2_WS;
    } else {
        return -1;
    }
    /* Stop bits */
    if (stop_bits == 2) {
        cr2 |= UART_CR2_STPB;
    } else if (stop_bits == 1) {
        cr2 &= ~UART_CR2_STPB;
    } else {
        return -1;
    }
    /* Parity */
    if (parity == PARITY_NONE) {
        cr2 &= ~UART_CR2_PREN;
    } else if (parity == PARITY_ODD) {
        /* ODD */
        cr2 |= UART_CR2_PREN;
        cr2 |= UART_CR2_PROE;
    } else if (parity == PARITY_EVEN) {
        /* Even */
        cr2 |= UART_CR2_PREN;
        cr2 &= ~UART_CR2_PROE;
    } else {
        return -1;
    }
    /* Apply the changes */
    regs->cr2 = cr2;
    /* Now set the baud rate */
    imx_uart_regs_set_baud(regs, bps);
    return 0;
}

bool imx_uart_regs_is_tx_fifo_busy(
        imx_uart_regs_t *regs
) {
    /* check the TXFE (transmit buffer FIFO empty) flag, which is cleared
     * automatically when data is written to the TxFIFO. Even though the flag
     * is set, the actual data transmission via the UART's 32 byte FIFO buffer
     * might still be in progress.
     */
    return (0 == (regs->sr2 & UART_SR2_TXFIFO_EMPTY));
}




#pragma once

#include <stdbool.h>

#include "imx_uart_regs.h"

typedef struct imx_uart imx_uart_t;
volatile struct imx_uart {
    volatile imx_uart_regs_t *regs;
    /* Will be `true` if user wants `\n` chars to be automatically converted to
     * `\r\n` and will be `false` otherwise. */
    bool auto_insert_carriage_return;
};

/**
 * Initialises the IMX Uart device.
 * @param imx_uart
 * @param base_vaddr
 * @param auto_insert_carriage_return Set to `true` if you want `\n` chars to be
 * automatically converted to `\r\n` and set to `false` otherwise.
 * @return
 */
bool imx_uart_init(
        imx_uart_t *imx_uart,
        uintptr_t base_vaddr,
        bool auto_insert_carriage_return
);

/**
 * Prints out a character using the imx uart device.
 * @param imx_uart
 * @param c
 * @return
 */
int imx_uart_put_char(
        imx_uart_t *imx_uart,
        int c
);

/**
 * Gets character from UART device.
 * @param imx_uart
 * @return
 */
int imx_uart_get_char(
        imx_uart_t *imx_uart
);

#pragma once

#include <sel4cp.h>
#include <sel4/sel4.h>

#include "imx_uart.h"

#define IRQ_59_CHANNEL (2)

#define SERIAL_DRIVER_TO_SERIAL_CLIENT_CHANNEL (4)

typedef struct serial_driver serial_driver_t;
struct serial_driver {
    /* UART device. */
    imx_uart_t imx_uart;
};

//void serial_write(const char *s);

#pragma once

#include <sel4cp.h>
#include <sel4/sel4.h>

#include "imx_uart.h"
#include "shared_ringbuffer.h"
#include "shared_dma.h"

#define IRQ_59_CHANNEL (2)

#define SERIAL_DRIVER_TO_SERIAL_CLIENT_CHANNEL (4)

typedef struct serial_driver serial_driver_t;
struct serial_driver {
    /* UART device. */
    imx_uart_t imx_uart;
    /* Transaction ring buffer handle. This is a convenience `struct` that
     * contains all the pointers to the relevant "available" and "used" buffers. */
    ring_handle_t tx_ring_buf_handle;
};

//void serial_write(const char *s);

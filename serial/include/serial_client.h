#pragma once

#include <sel4cp.h>
#include <sel4/sel4.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "shared_ringbuffer.h"

#define SERIAL_CLIENT_TO_SERIAL_DRIVER_CHANNEL (3)

typedef struct serial_client serial_client_t;
struct serial_client {
    /* Ring buffer from transmitting characters to the `serial_driver`. */
    ring_handle_t tx_ring_buf;
};



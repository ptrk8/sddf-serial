#pragma once

#include <sel4cp.h>
#include <sel4/sel4.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "shared_ringbuffer.h"

#define SERIAL_CLIENT_TO_SERIAL_DRIVER_CHANNEL (3)

#define NUM_BUFFERS 512
#define BUF_SIZE 2048

typedef struct serial_client serial_client_t;
struct serial_client {
    /* Transaction ring buffer handle. This is a convenience `struct` that
     * contains all the pointers to the relevant "available" and "used" buffers. */
    ring_handle_t tx_ring_buf_handle;
};



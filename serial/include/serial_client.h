#pragma once

#include <sel4cp.h>
#include <sel4/sel4.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "shared_ringbuffer.h"
#include "shared_dma.h"

#define SERIAL_CLIENT_TO_SERIAL_DRIVER_CHANNEL (3)

typedef struct serial_client serial_client_t;
struct serial_client {
    /* Transaction ring buffer handle. This is a convenience `struct` that
     * contains all the pointers to the relevant "available" and "used" buffers. */
    ring_handle_t tx_ring_buf_handle;
};

/**
 * Converts a format string into a normal string and saves it in the `str` buffer.
 * @param str Buffer to save the formatted string into.
 * @param maxlen Length of the `str` buffer.
 * @param format Format string.
 * @param args Arguments for the format string.
 * @return -1 if error and the number of characters written to `str` excluding
 * NULL terminator otherwise.
 */
static int serial_client_vsnprintf(
        char *str,
        size_t maxlen,
        const char *format,
        va_list args
);

//extern int vsnprintf (char *__restrict __s, size_t __maxlen,
//                      const char *__restrict __format, _G_va_list __arg)
//__THROWNL __attribute__ ((__format__ (__printf__, 3, 0)));



#include "serial_client.h"

/* TODO: Explain. */
uintptr_t tx_avail_ring_buf;
/* TODO: Explain. */
uintptr_t tx_used_ring_buf;
/* TODO: Explain. */
uintptr_t shared_dma;

/* Global serial client. */
serial_client_t global_serial_client = {0};

/**
 * Prints a string.
 * @param str
 */
static void serial_client_printf(char *str);

/**
 * Notifies the `serial_driver` PD.
 */
static void serial_client_notify_serial_driver();

static void serial_client_notify_serial_driver() {
    sel4cp_notify(SERIAL_CLIENT_TO_SERIAL_DRIVER_CHANNEL);
}

//int printf(const char *format, ...) {
//
//    /* Declare a va_list type variable */
//    va_list arguments;
//    /* Initialise the va_list variable with the ... after fmt */
//    va_start(arguments, format);
//
//    return 0;
//}
//
//int getchar(void) {
//    return 0;
//}

static void serial_client_printf(char *str) {
    /* Local reference to global serial_client for convenience. */
    serial_client_t *serial_client = &global_serial_client;
    /* The dequeued buffer's address will be stored in `buf_addr`. */
    uintptr_t buf_addr;
    /* The dequeued buffer's length will be stored in `buf_len`. */
    unsigned int buf_len;
    /* We don't use the `cookie` but the `dequeue_avail` function call requires
     * a valid pointer for the `cookie` param, so we provide one to it anyway. */
    void *unused_cookie;
    /* Dequeue an available buffer. */
    int ret_dequeue_avail = dequeue_avail(
            &serial_client->tx_ring_buf_handle,
            &buf_addr,
            &buf_len,
            &unused_cookie
    );
    if (ret_dequeue_avail < 0) {
        sel4cp_dbg_puts("Failed to dequeue buffer from available queue in serial_client_printf().\n");
        return;
    }
    /* Length of string including the NULL terminator. */
    size_t str_len = strlen(str) + 1;
    /* Copy the string (including the NULL terminator) into
     * `buf_addr` to update our dequeued available buffer. */
    memcpy(
            (char *) buf_addr,
            str,
            /* We define the length of a string as inclusive of its NULL terminator. */
            str_len
    );
    /* Since we have just written fresh data to the `shared_dma` memory region,
     * we need to clean the cache, which will force the contents of the cache to
     * be written back to physical memory. This ensures subsequent reads of the
     * `shared_dma` memory region will contain the latest data we have just
     * written. */
    int ret_vspace_clean = seL4_ARM_VSpace_Clean_Data(
            3, /* */
            buf_addr,
            buf_addr + str_len
    );
    if (ret_vspace_clean) {
        sel4cp_dbg_puts("Failed to clean cache in serial_client_printf().\n");
        return;
    }
    /* Enqueue our dequeued (and now updated) available buffer into the used
     * transmit queue. Now the `serial_driver` PD will be able to access our
     * buffer by de-queuing from this used transmit queue. */
    int ret_enqueue_used = enqueue_used(
            &serial_client->tx_ring_buf_handle,
            buf_addr,
            str_len,
            unused_cookie
    );
    if (ret_enqueue_used < 0) {
        sel4cp_dbg_puts("Transmit used ring is full in serial_client_printf().\n");
        return;
    }
    /* Notify the `serial_driver`. Since, we have a lower priority than the
     * `serial_driver`, we will be pre-empted after the call to
     * `sel4cp_notify()` until the `serial_driver` has finished printing
     * characters to the screen. */
    serial_client_notify_serial_driver();
}

void init(void) {
    /* Local reference to global serial_client for convenience. */
    serial_client_t *serial_client = &global_serial_client;
    /* Initialise our `tx_ring_buf_handle`, which is just a convenience struct
     * where all relevant ring buffers are located. */
    ring_init(
            &serial_client->tx_ring_buf_handle,
            (ring_buffer_t *) tx_avail_ring_buf,
            (ring_buffer_t *) tx_used_ring_buf,
            NULL,
            1
    );
    /* Populate the available ring buffer with empty buffers from `shared_dma`
     * memory region. */
    for (int i = 0; i < NUM_BUFFERS - 1; i++) {
        int ret_enqueue_avail = enqueue_avail(
                &serial_client->tx_ring_buf_handle,
                shared_dma + (BUF_SIZE * i),
                BUF_SIZE,
                NULL
        );
        if (ret_enqueue_avail < 0) {
            sel4cp_dbg_puts("Failed to enqueue buffer onto available queue.\n");
            return;
        }
    }

    serial_client_printf("\n=== START ===\n");
    serial_client_printf("Initialising UART device...\n");
    serial_client_printf("UART device initialisation SUCCESS.\n");
    serial_client_printf("\n");
    for (int i = 0; i < 5; i++) {
        serial_client_printf("a");
        serial_client_printf("b");
        serial_client_printf("c");
    }
    serial_client_printf("\n");
    serial_client_printf("Ending UART test...\n");
    serial_client_printf("=== END ===\n");
}

seL4_MessageInfo_t protected(sel4cp_channel ch, sel4cp_msginfo msginfo) {
    return sel4cp_msginfo_new(0, 0);
}

void notified(sel4cp_channel channel) {

}

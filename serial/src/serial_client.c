
#include "serial_client.h"

/* This is the shared buffer we write to and the `serial_driver` PD reads from.
 * */
uintptr_t serial_to_client_transmit_buf;

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
     * a valid pointer for the `cookie` param. */
    void *unused_cookie;
    /* Dequeue an available buffer. */
    int ret_dequeue_avail = dequeue_avail(
            &serial_client->tx_ring_buf,
            &buf_addr,
            &buf_len,
            &unused_cookie
    );
    if (ret_dequeue_avail < 0) {
        sel4cp_dbg_puts("Failed to dequeue buffer from available queue.\n");
        return;
    }
    /* Copy the string (including the NULL terminator) into
     * `buf_addr`. */
    memcpy(
            (char *) buf_addr,
            str,
            /* We define the length of a string as inclusive of its NULL terminator. */
            strlen(str) + 1
    );
    /* TODO: Get rid of this once we transition to using sDDF buffers. */
    /* Copy the string (including the NULL terminator) into
     * `serial_to_client_transmit_buf`. */
    memcpy(
            (char *) serial_to_client_transmit_buf,
            str,
            /* We define the length of a string as inclusive of its NULL terminator. */
            strlen(str) + 1
    );
    /* Notify the `serial_driver`. Since, we have a lower priority than the
     * `serial_driver`, we will be pre-empted after the call to
     * `sel4cp_notify()` until the `serial_driver` has finished printing
     * characters to the screen. */
    serial_client_notify_serial_driver();
}

void init(void) {
    /* Local reference to global serial_client for convenience. */
    serial_client_t *serial_client = &global_serial_client;
    /* Initialise `tx_ring_buf`. */
    ring_init(
            &serial_client->tx_ring_buf,
            (ring_buffer_t *) tx_avail_ring_buf,
            (ring_buffer_t *) tx_used_ring_buf,
            NULL,
            1
    );
    /* Populate the available ring buffer with empty buffers from `shared_dma`
     * memory region. */
    for (int i = 0; i < NUM_BUFFERS - 1; i++) {
        int ret_enqueue_avail = enqueue_avail(
                &serial_client->tx_ring_buf,
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

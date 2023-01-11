
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
 * Initialises the `serial_client` struct and sets up the serial client.
 * @param serial_client
 * @return 0 for success and -1 for error.
 */
static int serial_client_init(
        serial_client_t *serial_client
);

/**
 * Prints a string.
 * @param str
 */
static void serial_client_printf(char *str);

/**
 * Notifies the `serial_driver` PD.
 */
static void serial_client_notify_serial_driver();

static int serial_client_init(
        serial_client_t *serial_client
) {
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
            return -1;
        }
    }
    return 0;
}

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
    if (str_len > BUF_SIZE) {
        /* TODO: Allocate multiple buffers for strings larger than BUF_SIZE. */
        return;
    }
    /* Copy the string (including the NULL terminator) into
     * `buf_addr` to update our dequeued available buffer. */
    memcpy(
            (char *) buf_addr,
            str,
            str_len /* We define the length of a string as inclusive of its NULL
            terminator. */
    );
    /* Since we have just written fresh data to the `shared_dma` memory region,
     * we need to clean the cache, which will force the contents of the cache to
     * be written back to physical memory. This ensures subsequent reads of the
     * `shared_dma` memory region will contain the latest data we have just
     * written. */
    int ret_vspace_clean = seL4_ARM_VSpace_Clean_Data(
            3, /* The capability to every PD's VSpace. */
            buf_addr,
            buf_addr + str_len
    );
    if (ret_vspace_clean) {
        sel4cp_dbg_puts("Failed to clean cache in serial_client_printf().\n");
        return;
    }
    /* If the Transmit-Used ring was empty prior to us enqueuing our new used
     * buffer to the Transmit-Used ring (done in the following step), then we
     * know the `serial_driver` PD was NOT preempted while working on the
     * Transmit-Used ring. Therefore, after we enqueue our buffer to the
     * Transmit-Used buffer, we need to notify the `serial_driver` PD.
     * Conversely, if the Transmit-Used ring was NOT empty, then we know the
     * `serial_driver` PD was preempted while doing work on the Transmit-Used
     * ring. Therefore, we can save the kernel an unnecessary system call by NOT
     * notifying the `serial_driver` PD since the kernel will resume the
     * `serial_driver` PD (e.g. once its budget is refilled). This optimisation
     * is handled by the if(tx_used_was_empty) check later in this function. */
    bool tx_used_was_empty = ring_empty(serial_client->tx_ring_buf_handle.used_ring);
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
    /* As explained above, we only notify the `serial_driver` PD if the
     * Transmit-Used buffer was empty, which we know is when the PD was NOT
     * doing `printf` work for us. In our current setup (with no scheduling
     * budget or period set), this optimisation is unnecessary because our PD
     * has a lower priority than the `serial_driver` PD, meaning the
     * `serial_driver` PD can never be preempted i.e. the `serial_driver` PD
     * will always clear all the buffers in the Transmit-Used ring before we are
     * allowed to run. However, this optimisation becomes necessary if we
     * eventually decide to ascribe a scheduling "budget" and "period" to the
     * higher-priority `serial_driver` PD (which is the case for the ethernet
     * driver), which would allow the `serial_driver` PD to be preempted by us
     * (leading to several buffers left in the Transmit-Used buffer for the
     * `serial_driver` PD to resume processing once its scheduling budget is
     * refilled at some later time). */
    if (tx_used_was_empty) {
        /* Notify the `serial_driver`. */
        serial_client_notify_serial_driver();
    }
}

static void serial_client_notify_serial_driver() {
    sel4cp_notify(SERIAL_CLIENT_TO_SERIAL_DRIVER_CHANNEL);
}

void init(void) {
    /* Local reference to global serial_client for convenience. */
    serial_client_t *serial_client = &global_serial_client;
    /* Initialise serial_client. */
    int ret_serial_client_init = serial_client_init(serial_client);
    if (ret_serial_client_init < 0) {
        sel4cp_dbg_puts("Serial Client initialisation FAILED.\n");
        return;
    }
    /* Basic E2E tests for printf. */
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

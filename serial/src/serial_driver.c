#include "serial_driver.h"

/* This will be automatically populated with the Virtual Address that
 * corresponds to the Physical Address of the UART device by the seL4CP tool. */
uintptr_t uart_base_vaddr;

/* This is the buffer we read from after `serial_client` writes to it. */
uintptr_t serial_to_client_transmit_buf;

/* TODO: Explain. */
uintptr_t tx_avail_ring_buf;
/* TODO: Explain. */
uintptr_t tx_used_ring_buf;
/* TODO: Explain. */
uintptr_t shared_dma;

serial_driver_t global_serial_driver = {};

/**
 * Initialises the serial driver. Called by this PD's `init` function.
 * @param serial_driver
 * @param imx_uart_base_vaddr
 * @param auto_insert_carriage_return
 * @return
 */
static int serial_driver_init(
        serial_driver_t *serial_driver,
        uintptr_t imx_uart_base_vaddr,
        bool auto_insert_carriage_return
);

//void serial_write(const char *str) {
//    while (*str) {
////        uart_put_char(&uart, *str);
//        str++;
//    }
//}

static int serial_driver_init(
        serial_driver_t *serial_driver,
        uintptr_t imx_uart_base_vaddr,
        bool auto_insert_carriage_return
) {
    bool is_success = imx_uart_init(
            &serial_driver->imx_uart,
            imx_uart_base_vaddr,
            auto_insert_carriage_return
    );
    if (is_success) {
        return 0;
    }
    return -1;
}

void serial_driver_put_char(serial_driver_t *serial_driver, int ch) {
    /* Keep trying to send the character to the UART device until it is successful. */
    while (imx_uart_put_char(&serial_driver->imx_uart, ch) < 0);
}

void init(void) {
//    sel4cp_dbg_puts("\n=== START ===\n");
//    sel4cp_dbg_puts("Initialising UART device...\n");
    serial_driver_t *serial_driver = &global_serial_driver; /* Local reference to global serial driver for our convenience. */
    int ret_serial_driver_init = serial_driver_init(
            serial_driver,
            uart_base_vaddr,
            true
    );
    if (ret_serial_driver_init < 0) {
        sel4cp_dbg_puts("UART device initialisation FAILED.\n");
        return;
    }
//    sel4cp_dbg_puts("UART device initialisation SUCCESS.\n");
//    serial_driver_put_char(serial_driver, '\n');
//    for (int i = 0; i < 5; i++) {
//        serial_driver_put_char(serial_driver, 'a');
//        serial_driver_put_char(serial_driver, 'b');
//        serial_driver_put_char(serial_driver, 'c');
//    }
//    serial_driver_put_char(serial_driver, '\n');
//    sel4cp_dbg_puts("Ending UART test...\n");
//    sel4cp_dbg_puts("=== END ===\n");
}

seL4_MessageInfo_t protected(sel4cp_channel ch, sel4cp_msginfo msginfo) {
    return sel4cp_msginfo_new(0, 0);
}

void notified(sel4cp_channel channel) {
    serial_driver_t *serial_driver = &global_serial_driver; /* Local reference to global serial driver for our convenience. */
    switch(channel) {
        /* This is triggered when there is UART hardware interrupt (signifying a
         * new character was sent to the UART device). */
        case IRQ_59_CHANNEL: {
            /* We obtain the character for the UART device. */
            int c = imx_uart_get_char(&global_serial_driver.imx_uart);
            /* If the character is not erroneous, we send the character to the
             * UART device to output to the console. */
            if (c != -1) {
                serial_driver_put_char(serial_driver, c);
            }
            /* Acknowledge receipt of the interrupt. */
            sel4cp_irq_ack(channel);
            return;
        }
        /* This is triggered when `serial_client` wants to `printf` something. */
        case SERIAL_DRIVER_TO_SERIAL_CLIENT_CHANNEL: {
            int curr_idx = 0;
            /* Keep looping and printing out the characters until you hit a NULL terminator. */
            while (((char *) serial_to_client_transmit_buf)[curr_idx] != '\0') {
                /* Print the character out to the terminal. */
                serial_driver_put_char(
                        serial_driver,
                        ((char *) serial_to_client_transmit_buf)[curr_idx]
                );
                curr_idx++;
            }
            /* Note, we deliberately do NOT print out the NULL terminator. */
            return;
        }
        default:
            sel4cp_dbg_puts("Serial driver: received notification on unexpected channel\n");
            break;
    }
}

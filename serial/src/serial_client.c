
#include "serial_client.h"

/* This is the buffer we write to and `serial_driver` reads from. */
uintptr_t serial_to_client_transmit_buf;

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
     * `sel4cp_notify()` until the `serial_driver` has finished printing the
     * character to the screen. */
    serial_client_notify_serial_driver();
}

void init(void) {
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

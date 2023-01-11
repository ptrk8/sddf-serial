#include "gtest/gtest.h"

extern "C" {
#include <serial_client.h>
}

/* serial_client_vsnprintf */

TEST(test_serial_client, serial_client_vsnprintf_should_return_error_if_NULL_str) {
    va_list args;
    int ret = serial_client_vsnprintf(NULL, 100, "", args);
    ASSERT_EQ(-1, ret);
}




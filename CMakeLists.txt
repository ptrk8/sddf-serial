
add_subdirectory(serial/)

# ====================================
# Enables unit testing
# ====================================

include (CTest)
# Enabling tests
enable_testing()
# Add our test directory.
add_subdirectory(serial_test/unit)

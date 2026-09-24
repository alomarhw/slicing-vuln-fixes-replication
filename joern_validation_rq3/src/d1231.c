static enum test_return test_binary_incr(void) {
    return test_binary_incr_impl("test_binary_incr",
                                 PROTOCOL_BINARY_CMD_INCREMENT);
}

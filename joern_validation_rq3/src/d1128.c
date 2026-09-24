void PrintToStderr(const char* output) {
  ignore_result(HANDLE_EINTR(write(STDERR_FILENO, output, strlen(output))));
}

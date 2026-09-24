static void usage(const char *name) {
  printf("Usage: %s <command> [options]\n", name);
  printf("Commands:\n");
 for (size_t i = 0; i < ARRAY_SIZE(commands); ++i)
    printf("  %s\n", commands[i].name);
  printf("For detailed help on a command, run '%s help <command>'.\n", name);
}

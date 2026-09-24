int ask_string(char **ret, const char *text, ...) {
        int r;

        assert(ret);
        assert(text);

        for (;;) {
                _cleanup_free_ char *line = NULL;
                va_list ap;

                if (colors_enabled())
                        fputs(ANSI_HIGHLIGHT, stdout);

                va_start(ap, text);
                vprintf(text, ap);
                va_end(ap);

                if (colors_enabled())
                        fputs(ANSI_NORMAL, stdout);

                fflush(stdout);

                r = read_line(stdin, LONG_LINE_MAX, &line);
                if (r < 0)
                        return r;
                if (r == 0)
                        return -EIO;

                if (!isempty(line)) {
                        *ret = TAKE_PTR(line);
                        return 0;
                }
        }
}

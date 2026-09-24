int vt_default_utf8(void) {
        _cleanup_free_ char *b = NULL;
        int r;

        /* Read the default VT UTF8 setting from the kernel */

        r = read_one_line_file("/sys/module/vt/parameters/default_utf8", &b);
        if (r < 0)
                return r;

         return parse_boolean(b);
 }

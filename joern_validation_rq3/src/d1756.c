local size_t num(char *arg)
{
    char *str = arg;
    size_t val = 0;

    if (*str == 0)
        bail("internal error: empty parameter", "");
    do {
        if (*str < '0' || *str > '9' ||
            (val && ((~(size_t)0) - (*str - '0')) / val < 10))
            bail("invalid numeric parameter: ", arg);
        val = val * 10 + (*str - '0');
    } while (*++str);
    return val;
}

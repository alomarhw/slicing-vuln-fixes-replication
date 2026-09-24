crm_xml_escape_shuffle(char *text, int start, int *length, const char *replace)
{
    int lpc;
    int offset = strlen(replace) - 1;   /* We have space for 1 char already */

    *length += offset;
    text = realloc_safe(text, *length);

    for (lpc = (*length) - 1; lpc > (start + offset); lpc--) {
        text[lpc] = text[lpc - offset];
    }

    memcpy(text + start, replace, offset + 1);
    return text;
}

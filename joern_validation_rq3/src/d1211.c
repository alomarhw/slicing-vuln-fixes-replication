htmlParseLookupSequence(htmlParserCtxtPtr ctxt, xmlChar first,
                        xmlChar next, xmlChar third, int iscomment,
                        int ignoreattrval)
{
    int base, len;
    htmlParserInputPtr in;
    const xmlChar *buf;
    int incomment = 0;
    int invalue = 0;
    char valdellim = 0x0;

    in = ctxt->input;
    if (in == NULL)
        return (-1);

    base = in->cur - in->base;
    if (base < 0)
        return (-1);

    if (ctxt->checkIndex > base)
        base = ctxt->checkIndex;

    if (in->buf == NULL) {
        buf = in->base;
        len = in->length;
    } else {
        buf = xmlBufContent(in->buf->buffer);
        len = xmlBufUse(in->buf->buffer);
    }

    /* take into account the sequence length */
    if (third)
        len -= 2;
    else if (next)
        len--;
    for (; base < len; base++) {
        if ((!incomment) && (base + 4 < len) && (!iscomment)) {
            if ((buf[base] == '<') && (buf[base + 1] == '!') &&
                (buf[base + 2] == '-') && (buf[base + 3] == '-')) {
                incomment = 1;
                /* do not increment past <! - some people use <!--> */
                base += 2;
            }
        }
        if (ignoreattrval) {
            if (buf[base] == '"' || buf[base] == '\'') {
                if (invalue) {
                    if (buf[base] == valdellim) {
                        invalue = 0;
                        continue;
                    }
                } else {
                    valdellim = buf[base];
                    invalue = 1;
                    continue;
                }
            } else if (invalue) {
                continue;
            }
        }
        if (incomment) {
            if (base + 3 > len)
                return (-1);
            if ((buf[base] == '-') && (buf[base + 1] == '-') &&
                (buf[base + 2] == '>')) {
                incomment = 0;
                base += 2;
            }
            continue;
        }
        if (buf[base] == first) {
            if (third != 0) {
                if ((buf[base + 1] != next) || (buf[base + 2] != third))
                    continue;
            } else if (next != 0) {
                if (buf[base + 1] != next)
                    continue;
            }
            ctxt->checkIndex = 0;
#ifdef DEBUG_PUSH
            if (next == 0)
                xmlGenericError(xmlGenericErrorContext,
                                "HPP: lookup '%c' found at %d\n",
                                first, base);
            else if (third == 0)
                xmlGenericError(xmlGenericErrorContext,
                                "HPP: lookup '%c%c' found at %d\n",
                                first, next, base);
            else
                xmlGenericError(xmlGenericErrorContext,
                                "HPP: lookup '%c%c%c' found at %d\n",
                                first, next, third, base);
#endif
            return (base - (in->cur - in->base));
        }
    }
    if ((!incomment) && (!invalue))
        ctxt->checkIndex = base;
#ifdef DEBUG_PUSH
    if (next == 0)
        xmlGenericError(xmlGenericErrorContext,
                        "HPP: lookup '%c' failed\n", first);
    else if (third == 0)
        xmlGenericError(xmlGenericErrorContext,
                        "HPP: lookup '%c%c' failed\n", first, next);
    else
        xmlGenericError(xmlGenericErrorContext,
                        "HPP: lookup '%c%c%c' failed\n", first, next,
                        third);
#endif
    return (-1);
}

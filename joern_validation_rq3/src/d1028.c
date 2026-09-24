exsltStrReplaceFunction (xmlXPathParserContextPtr ctxt, int nargs) {
    int i, i_empty, n, slen0, rlen0, *slen, *rlen;
    void *mem = NULL;
    const xmlChar *src, *start;
    xmlChar *string, *search_str = NULL, *replace_str = NULL;
    xmlChar **search, **replace;
    xmlNodeSetPtr search_set = NULL, replace_set = NULL;
    xmlBufferPtr buf;

    if (nargs  != 3) {
        xmlXPathSetArityError(ctxt);
        return;
    }

    /* get replace argument */

    if (!xmlXPathStackIsNodeSet(ctxt))
        replace_str = xmlXPathPopString(ctxt);
    else
        replace_set = xmlXPathPopNodeSet(ctxt);

    if (xmlXPathCheckError(ctxt))
        goto fail_replace;

    /* get search argument */

    if (!xmlXPathStackIsNodeSet(ctxt)) {
        search_str = xmlXPathPopString(ctxt);
        n = 1;
    }
    else {
        search_set = xmlXPathPopNodeSet(ctxt);
        n = search_set != NULL ? search_set->nodeNr : 0;
    }

    if (xmlXPathCheckError(ctxt))
        goto fail_search;

    /* get string argument */

    string = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt))
        goto fail_string;

    /* check for empty search node list */

    if (n <= 0) {
        exsltStrReturnString(ctxt, string, xmlStrlen(string));
        goto done_empty_search;
    }

    /* allocate memory for string pointer and length arrays */

    if (n == 1) {
        search = &search_str;
        replace = &replace_str;
        slen = &slen0;
        rlen = &rlen0;
    }
    else {
        mem = xmlMalloc(2 * n * (sizeof(const xmlChar *) + sizeof(int)));
        if (mem == NULL) {
            xmlXPathSetError(ctxt, XPATH_MEMORY_ERROR);
            goto fail_malloc;
        }
        search = (xmlChar **) mem;
        replace = search + n;
        slen = (int *) (replace + n);
        rlen = slen + n;
    }

    /* process arguments */

    i_empty = -1;

    for (i=0; i<n; ++i) {
        if (search_set != NULL) {
            search[i] = xmlXPathCastNodeToString(search_set->nodeTab[i]);
            if (search[i] == NULL) {
                n = i;
                goto fail_process_args;
            }
        }

        slen[i] = xmlStrlen(search[i]);
        if (i_empty < 0 && slen[i] == 0)
            i_empty = i;

        if (replace_set != NULL) {
            if (i < replace_set->nodeNr) {
                replace[i] = xmlXPathCastNodeToString(replace_set->nodeTab[i]);
                if (replace[i] == NULL) {
                    n = i + 1;
                    goto fail_process_args;
                }
            }
            else
                replace[i] = NULL;
        }
        else {
            if (i == 0)
                replace[i] = replace_str;
            else
                replace[i] = NULL;
        }

        if (replace[i] == NULL)
            rlen[i] = 0;
        else
            rlen[i] = xmlStrlen(replace[i]);
    }

    if (i_empty >= 0 && rlen[i_empty] == 0)
        i_empty = -1;

    /* replace operation */

    buf = xmlBufferCreate();
    if (buf == NULL) {
        xmlXPathSetError(ctxt, XPATH_MEMORY_ERROR);
        goto fail_buffer;
    }
    src = string;
    start = string;

    while (*src != 0) {
        int max_len = 0, i_match = 0;

        for (i=0; i<n; ++i) {
            if (*src == search[i][0] &&
                slen[i] > max_len &&
                xmlStrncmp(src, search[i], slen[i]) == 0)
            {
                i_match = i;
                max_len = slen[i];
            }
        }

        if (max_len == 0) {
            if (i_empty >= 0 && start < src) {
                if (xmlBufferAdd(buf, start, src - start) ||
                    xmlBufferAdd(buf, replace[i_empty], rlen[i_empty]))
                {
                    xmlXPathSetError(ctxt, XPATH_MEMORY_ERROR);
                    goto fail_buffer_add;
                }
                start = src;
            }

            src += xmlUTF8Size(src);
        }
        else {
            if ((start < src &&
                 xmlBufferAdd(buf, start, src - start)) ||
                (rlen[i_match] &&
                 xmlBufferAdd(buf, replace[i_match], rlen[i_match])))
            {
                xmlXPathSetError(ctxt, XPATH_MEMORY_ERROR);
                goto fail_buffer_add;
            }

            src += slen[i_match];
            start = src;
        }
    }

    if (start < src && xmlBufferAdd(buf, start, src - start)) {
        xmlXPathSetError(ctxt, XPATH_MEMORY_ERROR);
        goto fail_buffer_add;
    }

    /* create result node set */

    exsltStrReturnString(ctxt, xmlBufferContent(buf), xmlBufferLength(buf));

    /* clean up */

fail_buffer_add:
    xmlBufferFree(buf);

fail_buffer:
fail_process_args:
    if (search_set != NULL) {
        for (i=0; i<n; ++i)
            xmlFree(search[i]);
    }
    if (replace_set != NULL) {
        for (i=0; i<n; ++i) {
            if (replace[i] != NULL)
                xmlFree(replace[i]);
        }
    }

    if (mem != NULL)
        xmlFree(mem);

fail_malloc:
done_empty_search:
    xmlFree(string);

fail_string:
    if (search_set != NULL)
        xmlXPathFreeNodeSet(search_set);
    else
        xmlFree(search_str);

fail_search:
    if (replace_set != NULL)
        xmlXPathFreeNodeSet(replace_set);
    else
        xmlFree(replace_str);

fail_replace:
    return;
}

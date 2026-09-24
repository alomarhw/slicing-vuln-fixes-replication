htmlParseEndTag(htmlParserCtxtPtr ctxt)
{
    const xmlChar *name;
    const xmlChar *oldname;
    int i, ret;

    if ((CUR != '<') || (NXT(1) != '/')) {
        htmlParseErr(ctxt, XML_ERR_LTSLASH_REQUIRED,
	             "htmlParseEndTag: '</' not found\n", NULL, NULL);
        return (0);
    }
    SKIP(2);

    name = htmlParseHTMLName(ctxt);
    if (name == NULL)
        return (0);
    /*
     * We should definitely be at the ending "S? '>'" part
     */
    SKIP_BLANKS;
    if ((!IS_CHAR_CH(CUR)) || (CUR != '>')) {
        htmlParseErr(ctxt, XML_ERR_GT_REQUIRED,
	             "End tag : expected '>'\n", NULL, NULL);
	if (ctxt->recovery) {
	    /*
	     * We're not at the ending > !!
	     * Error, unless in recover mode where we search forwards
	     * until we find a >
	     */
	    while (CUR != '\0' && CUR != '>') NEXT;
	    NEXT;
	}
    } else
        NEXT;

    /*
     * if we ignored misplaced tags in htmlParseStartTag don't pop them
     * out now.
     */
    if ((ctxt->depth > 0) &&
        (xmlStrEqual(name, BAD_CAST "html") ||
         xmlStrEqual(name, BAD_CAST "body") ||
	 xmlStrEqual(name, BAD_CAST "head"))) {
	ctxt->depth--;
	return (0);
    }

    /*
     * If the name read is not one of the element in the parsing stack
     * then return, it's just an error.
     */
    for (i = (ctxt->nameNr - 1); i >= 0; i--) {
        if (xmlStrEqual(name, ctxt->nameTab[i]))
            break;
    }
    if (i < 0) {
        htmlParseErr(ctxt, XML_ERR_TAG_NAME_MISMATCH,
	             "Unexpected end tag : %s\n", name, NULL);
        return (0);
    }


    /*
     * Check for auto-closure of HTML elements.
     */

    htmlAutoCloseOnClose(ctxt, name);

    /*
     * Well formedness constraints, opening and closing must match.
     * With the exception that the autoclose may have popped stuff out
     * of the stack.
     */
    if (!xmlStrEqual(name, ctxt->name)) {
        if ((ctxt->name != NULL) && (!xmlStrEqual(ctxt->name, name))) {
            htmlParseErr(ctxt, XML_ERR_TAG_NAME_MISMATCH,
	                 "Opening and ending tag mismatch: %s and %s\n",
			 name, ctxt->name);
        }
    }

    /*
     * SAX: End of Tag
     */
    oldname = ctxt->name;
    if ((oldname != NULL) && (xmlStrEqual(oldname, name))) {
        if ((ctxt->sax != NULL) && (ctxt->sax->endElement != NULL))
            ctxt->sax->endElement(ctxt->userData, name);
	htmlNodeInfoPop(ctxt);
        htmlnamePop(ctxt);
        ret = 1;
    } else {
        ret = 0;
    }

    return (ret);
}

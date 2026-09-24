htmlAutoCloseOnClose(htmlParserCtxtPtr ctxt, const xmlChar * newtag)
{
    const htmlElemDesc *info;
    int i, priority;

    priority = htmlGetEndPriority(newtag);

    for (i = (ctxt->nameNr - 1); i >= 0; i--) {

        if (xmlStrEqual(newtag, ctxt->nameTab[i]))
            break;
        /*
         * A missplaced endtag can only close elements with lower
         * or equal priority, so if we find an element with higher
         * priority before we find an element with
         * matching name, we just ignore this endtag
         */
        if (htmlGetEndPriority(ctxt->nameTab[i]) > priority)
            return;
    }
    if (i < 0)
        return;

    while (!xmlStrEqual(newtag, ctxt->name)) {
        info = htmlTagLookup(ctxt->name);
        if ((info != NULL) && (info->endTag == 3)) {
            htmlParseErr(ctxt, XML_ERR_TAG_NAME_MISMATCH,
	                 "Opening and ending tag mismatch: %s and %s\n",
			 newtag, ctxt->name);
        }
        if ((ctxt->sax != NULL) && (ctxt->sax->endElement != NULL))
            ctxt->sax->endElement(ctxt->userData, ctxt->name);
	htmlnamePop(ctxt);
    }
}

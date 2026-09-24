xsltGetKey(xsltTransformContextPtr ctxt, const xmlChar *name,
	   const xmlChar *nameURI, const xmlChar *value) {
    xmlNodeSetPtr ret;
    xsltKeyTablePtr table;
    int init_table = 0;

    if ((ctxt == NULL) || (name == NULL) || (value == NULL) ||
	(ctxt->document == NULL))
	return(NULL);

#ifdef WITH_XSLT_DEBUG_KEYS
    xsltGenericDebug(xsltGenericDebugContext,
	"Get key %s, value %s\n", name, value);
#endif

    /*
     * keys are computed only on-demand on first key access for a document
     */
    if ((ctxt->document->nbKeysComputed < ctxt->nbKeys) &&
        (ctxt->keyInitLevel == 0)) {
        /*
	 * If non-recursive behaviour, just try to initialize all keys
	 */
	if (xsltInitAllDocKeys(ctxt))
	    return(NULL);
    }

retry:
    table = (xsltKeyTablePtr) ctxt->document->keys;
    while (table != NULL) {
	if (((nameURI != NULL) == (table->nameURI != NULL)) &&
	    xmlStrEqual(table->name, name) &&
	    xmlStrEqual(table->nameURI, nameURI))
	{
	    ret = (xmlNodeSetPtr)xmlHashLookup(table->keys, value);
	    return(ret);
	}
	table = table->next;
    }

    if ((ctxt->keyInitLevel != 0) && (init_table == 0)) {
        /*
	 * Apparently one key is recursive and this one is needed,
	 * initialize just it, that time and retry
	 */
        xsltInitDocKeyTable(ctxt, name, nameURI);
	init_table = 1;
	goto retry;
    }

    return(NULL);
}

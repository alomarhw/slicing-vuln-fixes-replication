xmlXPathFunctionLookup(xmlXPathContextPtr ctxt, const xmlChar *name) {
    if (ctxt == NULL)
	return (NULL);

    if (ctxt->funcLookupFunc != NULL) {
	xmlXPathFunction ret;
	xmlXPathFuncLookupFunc f;

	f = ctxt->funcLookupFunc;
	ret = f(ctxt->funcLookupData, name, NULL);
	if (ret != NULL)
	    return(ret);
    }
    return(xmlXPathFunctionLookupNS(ctxt, name, NULL));
}

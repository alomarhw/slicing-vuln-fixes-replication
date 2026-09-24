c14nRunTest(const char* xml_filename, int with_comments, int mode,
	    const char* xpath_filename, const char *ns_filename,
	    const char* result_file) {
    xmlDocPtr doc;
    xmlXPathObjectPtr xpath = NULL;
    xmlChar *result = NULL;
    int ret;
    xmlChar **inclusive_namespaces = NULL;
    const char *nslist = NULL;
    int nssize;


    /*
     * build an XML tree from a the file; we need to add default
     * attributes and resolve all character and entities references
     */
    xmlLoadExtDtdDefaultValue = XML_DETECT_IDS | XML_COMPLETE_ATTRS;
    xmlSubstituteEntitiesDefault(1);

    doc = xmlReadFile(xml_filename, NULL, XML_PARSE_DTDATTR | XML_PARSE_NOENT);
    if (doc == NULL) {
	fprintf(stderr, "Error: unable to parse file \"%s\"\n", xml_filename);
	return(-1);
    }

    /*
     * Check the document is of the right kind
     */
    if(xmlDocGetRootElement(doc) == NULL) {
        fprintf(stderr,"Error: empty document for file \"%s\"\n", xml_filename);
	xmlFreeDoc(doc);
	return(-1);
    }

    /*
     * load xpath file if specified
     */
    if(xpath_filename) {
	xpath = load_xpath_expr(doc, xpath_filename);
	if(xpath == NULL) {
	    fprintf(stderr,"Error: unable to evaluate xpath expression\n");
	    xmlFreeDoc(doc);
	    return(-1);
	}
    }

    if (ns_filename != NULL) {
        if (loadMem(ns_filename, &nslist, &nssize)) {
	    fprintf(stderr,"Error: unable to evaluate xpath expression\n");
	    if(xpath != NULL) xmlXPathFreeObject(xpath);
	    xmlFreeDoc(doc);
	    return(-1);
	}
        inclusive_namespaces = parse_list((xmlChar *) nslist);
    }

    /*
     * Canonical form
     */
    /* fprintf(stderr,"File \"%s\" loaded: start canonization\n", xml_filename); */
    ret = xmlC14NDocDumpMemory(doc,
	    (xpath) ? xpath->nodesetval : NULL,
	    mode, inclusive_namespaces,
	    with_comments, &result);
    if (ret >= 0) {
	if(result != NULL) {
	    if (compareFileMem(result_file, (const char *) result, ret)) {
		fprintf(stderr, "Result mismatch for %s\n", xml_filename);
		fprintf(stderr, "RESULT:\n%s\n", (const char*)result);
	        ret = -1;
	    }
	}
    } else {
	fprintf(stderr,"Error: failed to canonicalize XML file \"%s\" (ret=%d)\n", xml_filename, ret);
	ret = -1;
    }

    /*
     * Cleanup
     */
    if (result != NULL) xmlFree(result);
    if(xpath != NULL) xmlXPathFreeObject(xpath);
    if (inclusive_namespaces != NULL) xmlFree(inclusive_namespaces);
    if (nslist != NULL) free((char *) nslist);
    xmlFreeDoc(doc);

    return(ret);
}

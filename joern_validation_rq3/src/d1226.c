patternTest(const char *filename,
            const char *resul ATTRIBUTE_UNUSED,
            const char *err ATTRIBUTE_UNUSED,
            int options) {
    xmlPatternPtr patternc = NULL;
    xmlStreamCtxtPtr patstream = NULL;
    FILE *o, *f;
    char str[1024];
    char xml[500];
    char result[500];
    int len, i;
    int ret = 0, res;
    char *temp;
    xmlTextReaderPtr reader;
    xmlDocPtr doc;

    len = strlen(filename);
    len -= 4;
    memcpy(xml, filename, len);
    xml[len] = 0;
    snprintf(result, 499, "result/pattern/%s", baseFilename(xml));
    result[499] = 0;
    memcpy(xml + len, ".xml", 5);

    if (!checkTestFile(xml) && !update_results) {
	fprintf(stderr, "Missing xml file %s\n", xml);
	return(-1);
    }
    if (!checkTestFile(result) && !update_results) {
	fprintf(stderr, "Missing result file %s\n", result);
	return(-1);
    }
    f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "Failed to open %s\n", filename);
	return(-1);
    }
    temp = resultFilename(filename, "", ".res");
    if (temp == NULL) {
        fprintf(stderr, "Out of memory\n");
        fatalError();
    }
    o = fopen(temp, "wb");
    if (o == NULL) {
	fprintf(stderr, "failed to open output file %s\n", temp);
	fclose(f);
        free(temp);
	return(-1);
    }
    while (1) {
	/*
	 * read one line in string buffer.
	 */
	if (fgets (&str[0], sizeof (str) - 1, f) == NULL)
	   break;

	/*
	 * remove the ending spaces
	 */
	i = strlen(str);
	while ((i > 0) &&
	       ((str[i - 1] == '\n') || (str[i - 1] == '\r') ||
		(str[i - 1] == ' ') || (str[i - 1] == '\t'))) {
	    i--;
	    str[i] = 0;
	}
	doc = xmlReadFile(xml, NULL, options);
	if (doc == NULL) {
	    fprintf(stderr, "Failed to parse %s\n", xml);
	    ret = 1;
	} else {
	    xmlNodePtr root;
	    const xmlChar *namespaces[22];
	    int j;
	    xmlNsPtr ns;

	    root = xmlDocGetRootElement(doc);
	    for (ns = root->nsDef, j = 0;ns != NULL && j < 20;ns=ns->next) {
		namespaces[j++] = ns->href;
		namespaces[j++] = ns->prefix;
	    }
	    namespaces[j++] = NULL;
	    namespaces[j] = NULL;

	    patternc = xmlPatterncompile((const xmlChar *) str, doc->dict,
					 0, &namespaces[0]);
	    if (patternc == NULL) {
		testErrorHandler(NULL,
			"Pattern %s failed to compile\n", str);
		xmlFreeDoc(doc);
		ret = 1;
		continue;
	    }
	    patstream = xmlPatternGetStreamCtxt(patternc);
	    if (patstream != NULL) {
		ret = xmlStreamPush(patstream, NULL, NULL);
		if (ret < 0) {
		    fprintf(stderr, "xmlStreamPush() failure\n");
		    xmlFreeStreamCtxt(patstream);
		    patstream = NULL;
		}
	    }
	    nb_tests++;

	    reader = xmlReaderWalker(doc);
	    res = xmlTextReaderRead(reader);
	    while (res == 1) {
		patternNode(o, reader, str, patternc, patstream);
		res = xmlTextReaderRead(reader);
	    }
	    if (res != 0) {
		fprintf(o, "%s : failed to parse\n", filename);
	    }
	    xmlFreeTextReader(reader);
	    xmlFreeDoc(doc);
	    xmlFreeStreamCtxt(patstream);
	    patstream = NULL;
	    xmlFreePattern(patternc);

	}
    }

    fclose(f);
    fclose(o);

    ret = compareFiles(temp, result);
    if (ret) {
	fprintf(stderr, "Result for %s failed in %s\n", filename, result);
	ret = 1;
    }
    if (temp != NULL) {
        unlink(temp);
        free(temp);
    }
    return(ret);
}

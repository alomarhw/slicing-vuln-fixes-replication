xmlParseTextDecl(xmlParserCtxtPtr ctxt) {
    xmlChar *version;
 const xmlChar *encoding;

 /*
     * We know that '<?xml' is here.
     */
 if ((CMP5(CUR_PTR, '<', '?', 'x', 'm', 'l')) && (IS_BLANK_CH(NXT(5)))) {
	SKIP(5);
 } else {
	xmlFatalErr(ctxt, XML_ERR_XMLDECL_NOT_STARTED, NULL);
 return;
 }

 if (!IS_BLANK_CH(CUR)) {
	xmlFatalErrMsg(ctxt, XML_ERR_SPACE_REQUIRED,
 "Space needed after '<?xml'\n");
 }
    SKIP_BLANKS;

 /*
     * We may have the VersionInfo here.
     */
    version = xmlParseVersionInfo(ctxt);
 if (version == NULL)
	version = xmlCharStrdup(XML_DEFAULT_VERSION);
 else {
 if (!IS_BLANK_CH(CUR)) {
	    xmlFatalErrMsg(ctxt, XML_ERR_SPACE_REQUIRED,
 "Space needed here\n");
 }
 }
    ctxt->input->version = version;

 /*
     * We must have the encoding declaration
     */
    encoding = xmlParseEncodingDecl(ctxt);
 if (ctxt->errNo == XML_ERR_UNSUPPORTED_ENCODING) {
 /*
	 * The XML REC instructs us to stop parsing right here
	 */
 return;
 }
 if ((encoding == NULL) && (ctxt->errNo == XML_ERR_OK)) {
	xmlFatalErrMsg(ctxt, XML_ERR_MISSING_ENCODING,
 "Missing encoding in text declaration\n");
 }

    SKIP_BLANKS;
 if ((RAW == '?') && (NXT(1) == '>')) {
        SKIP(2);
 } else if (RAW == '>') {
 /* Deprecated old WD ... */
	xmlFatalErr(ctxt, XML_ERR_XMLDECL_NOT_FINISHED, NULL);
	NEXT;
 } else {
	xmlFatalErr(ctxt, XML_ERR_XMLDECL_NOT_FINISHED, NULL);
	MOVETO_ENDTAG(CUR_PTR);
	NEXT;
 }
}

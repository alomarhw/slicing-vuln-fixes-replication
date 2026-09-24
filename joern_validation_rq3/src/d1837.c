xmlParse3986Scheme(xmlURIPtr uri, const char **str) {
 const char *cur;

 if (str == NULL)
 return(-1);

    cur = *str;
 if (!ISA_ALPHA(cur))
 return(2);
    cur++;
 while (ISA_ALPHA(cur) || ISA_DIGIT(cur) ||
 (*cur == '+') || (*cur == '-') || (*cur == '.')) cur++;
 if (uri != NULL) {
 if (uri->scheme != NULL) xmlFree(uri->scheme);
	uri->scheme = STRNDUP(*str, cur - *str);
 }
 *str = cur;
 return(0);
}

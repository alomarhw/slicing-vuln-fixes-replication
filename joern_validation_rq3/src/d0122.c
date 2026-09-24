xmlXPathInit(void) {
    if (xmlXPathInitialized) return;

    xmlXPathPINF = trio_pinf();
    xmlXPathNINF = trio_ninf();
    xmlXPathNAN = trio_nan();
    xmlXPathNZERO = trio_nzero();

    xmlXPathInitialized = 1;
}

change_charset(struct parsed_tagarg *arg)
{
    Buffer *buf = Currentbuf->linkBuffer[LB_N_INFO];
    wc_ces charset;

    if (buf == NULL)
	return;
    delBuffer(Currentbuf);
    Currentbuf = buf;
    if (Currentbuf->bufferprop & BP_INTERNAL)
	return;
    charset = Currentbuf->document_charset;
    for (; arg; arg = arg->next) {
	if (!strcmp(arg->arg, "charset"))
	    charset = atoi(arg->value);
    }
    _docCSet(charset);
}

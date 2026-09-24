void init_decode_cache(struct x86_emulate_ctxt *ctxt)
{
	memset(&ctxt->rip_relative, 0,
	       (void *)&ctxt->modrm - (void *)&ctxt->rip_relative);

	ctxt->io_read.pos = 0;
	ctxt->io_read.end = 0;
	ctxt->mem_read.end = 0;
}

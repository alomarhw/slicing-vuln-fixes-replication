static pgprot_t __get_dma_pgprot(struct dma_attrs *attrs, pgprot_t prot,
				 bool coherent)
{
	if (!coherent || dma_get_attr(DMA_ATTR_WRITE_COMBINE, attrs))
		return pgprot_writecombine(prot);
	return prot;
}

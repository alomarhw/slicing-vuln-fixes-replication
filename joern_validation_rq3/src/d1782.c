static struct page **__atomic_get_pages(void *addr)
{
	struct dma_pool *pool = &atomic_pool;
	struct page **pages = pool->pages;
	int offs = (addr - pool->vaddr) >> PAGE_SHIFT;

	return pages + offs;
}

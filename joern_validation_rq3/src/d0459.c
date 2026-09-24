static inline int SlabFrozen(struct page *page)
{
	return page->flags & FROZEN;
}

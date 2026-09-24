static inline struct page *alloc_hugepage(int defrag)
{
	return alloc_pages(alloc_hugepage_gfpmask(defrag, 0),
			   HPAGE_PMD_ORDER);
}

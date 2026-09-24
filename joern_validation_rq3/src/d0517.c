void kvm_set_page_dirty(struct page *page)
{
	kvm_set_pfn_dirty(page_to_pfn(page));
}

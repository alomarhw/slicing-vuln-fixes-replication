struct page *gfn_to_page(struct kvm *kvm, gfn_t gfn)
{
	pfn_t pfn;

	pfn = gfn_to_pfn(kvm, gfn);
	if (!kvm_is_mmio_pfn(pfn))
		return pfn_to_page(pfn);

	WARN_ON(kvm_is_mmio_pfn(pfn));

	get_page(bad_page);
	return bad_page;
}

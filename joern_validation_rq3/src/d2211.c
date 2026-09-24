static void pte_to_pagemap_entry(pagemap_entry_t *pme, struct pagemapread *pm,
		struct vm_area_struct *vma, unsigned long addr, pte_t pte)
{
	u64 frame, flags;
	struct page *page = NULL;
	int flags2 = 0;

	if (pte_present(pte)) {
		frame = pte_pfn(pte);
		flags = PM_PRESENT;
		page = vm_normal_page(vma, addr, pte);
		if (pte_soft_dirty(pte))
			flags2 |= __PM_SOFT_DIRTY;
	} else if (is_swap_pte(pte)) {
		swp_entry_t entry;
		if (pte_swp_soft_dirty(pte))
			flags2 |= __PM_SOFT_DIRTY;
		entry = pte_to_swp_entry(pte);
		frame = swp_type(entry) |
			(swp_offset(entry) << MAX_SWAPFILES_SHIFT);
		flags = PM_SWAP;
		if (is_migration_entry(entry))
			page = migration_entry_to_page(entry);
	} else {
		if (vma->vm_flags & VM_SOFTDIRTY)
			flags2 |= __PM_SOFT_DIRTY;
		*pme = make_pme(PM_NOT_PRESENT(pm->v2) | PM_STATUS2(pm->v2, flags2));
		return;
	}

	if (page && !PageAnon(page))
		flags |= PM_FILE;
	if ((vma->vm_flags & VM_SOFTDIRTY))
		flags2 |= __PM_SOFT_DIRTY;

	*pme = make_pme(PM_PFRAME(frame) | PM_STATUS2(pm->v2, flags2) | flags);
}

static void update_last_pte_bitmap(struct kvm_vcpu *vcpu, struct kvm_mmu *mmu)
{
	u8 map;
	unsigned level, root_level = mmu->root_level;
	const unsigned ps_set_index = 1 << 2;  /* bit 2 of index: ps */

	if (root_level == PT32E_ROOT_LEVEL)
		--root_level;
	/* PT_PAGE_TABLE_LEVEL always terminates */
	map = 1 | (1 << ps_set_index);
	for (level = PT_DIRECTORY_LEVEL; level <= root_level; ++level) {
		if (level <= PT_PDPE_LEVEL
		    && (mmu->root_level >= PT32E_ROOT_LEVEL || is_pse(vcpu)))
			map |= 1 << (ps_set_index | (level - 1));
	}
	mmu->last_pte_bitmap = map;
}

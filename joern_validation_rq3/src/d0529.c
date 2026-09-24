void mem_cgroup_split_huge_fixup(struct page *head)
{
	struct page_cgroup *head_pc = lookup_page_cgroup(head);
	struct page_cgroup *pc;
	int i;

	if (mem_cgroup_disabled())
		return;
	for (i = 1; i < HPAGE_PMD_NR; i++) {
		pc = head_pc + i;
		pc->mem_cgroup = head_pc->mem_cgroup;
		smp_wmb();/* see __commit_charge() */
		pc->flags = head_pc->flags & ~PCGF_NOCOPY_AT_SPLIT;
	}
}

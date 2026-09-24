void page_add_file_rmap(struct page *page)
{
	bool locked;
	unsigned long flags;

	mem_cgroup_begin_update_page_stat(page, &locked, &flags);
	if (atomic_inc_and_test(&page->_mapcount)) {
		__inc_zone_page_state(page, NR_FILE_MAPPED);
		mem_cgroup_inc_page_stat(page, MEM_CGROUP_STAT_FILE_MAPPED);
	}
	mem_cgroup_end_update_page_stat(page, &locked, &flags);
}

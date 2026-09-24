struct minidump_memory_info *r_bin_mdmp_get_mem_info(struct r_bin_mdmp_obj *obj, ut64 vaddr) {
	struct minidump_memory_info *mem_info;
	RListIter *it;

	if (!obj) return NULL;

	r_list_foreach (obj->streams.memory_infos, it, mem_info) {
		if (mem_info->allocation_base && vaddr == mem_info->base_address) {
			return mem_info;
		}
	}

	return NULL;
}

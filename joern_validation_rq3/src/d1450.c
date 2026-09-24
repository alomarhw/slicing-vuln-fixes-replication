static RList* entries(RBinFile *bf) {
	struct Elf_(r_bin_elf_obj_t)* obj;
	RBinAddr *ptr = NULL;
	struct r_bin_elf_symbol_t *symbol;
	RList *ret;
	int i;

	if (!bf || !bf->o || !bf->o->bin_obj) {
		return NULL;
	}
	obj = bf->o->bin_obj;
	if (!(ret = r_list_newf ((RListFree)free))) {
		return NULL;
	}
	if (!(ptr = R_NEW0 (RBinAddr))) {
		return ret;
	}
	ptr->paddr = Elf_(r_bin_elf_get_entry_offset) (obj);
	ptr->vaddr = Elf_(r_bin_elf_p2v) (obj, ptr->paddr);
	ptr->haddr = 0x18;

	if (obj->ehdr.e_machine == EM_ARM) {
		int bin_bits = Elf_(r_bin_elf_get_bits) (obj);
		if (bin_bits != 64) {
			ptr->bits = 32;
			if (ptr->vaddr & 1) {
				ptr->vaddr--;
				ptr->bits = 16;
			}
			if (ptr->paddr & 1) {
				ptr->paddr--;
				ptr->bits = 16;
			}
		}
	}
	r_list_append (ret, ptr);

	if (!(symbol = Elf_(r_bin_elf_get_symbols) (obj))) {
		return ret;
	}
	for (i = 0; !symbol[i].last; i++) {
		if (!strncmp (symbol[i].name, "Java", 4)) {
			if (r_str_endswith (symbol[i].name, "_init")) {
				if (!(ptr = R_NEW0 (RBinAddr))) {
					return ret;
				}
				ptr->paddr = symbol[i].offset;
				ptr->vaddr = Elf_(r_bin_elf_p2v) (obj, ptr->paddr);
				ptr->haddr = UT64_MAX;
				ptr->type = R_BIN_ENTRY_TYPE_INIT;
				r_list_append (ret, ptr);
				break;
			}
		}
	}
	int bin_bits = Elf_(r_bin_elf_get_bits) (bf->o->bin_obj);
	process_constructors (bf, ret, bin_bits < 32 ? 32: bin_bits);
	return ret;
}

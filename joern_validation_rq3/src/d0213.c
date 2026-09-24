static TEE_Result set_seg_prot(struct user_ta_ctx *utc,
			       struct user_ta_elf *elf)
{
	TEE_Result res;
	size_t n;

	for (n = 0; n < elf->num_segs; n++) {
		struct load_seg *seg = &elf->segs[n];

		res = vm_set_prot(utc, seg->va, seg->size,
				  elf_flags_to_mattr(seg->flags));
		if (res)
			break;
	}
	return res;
}

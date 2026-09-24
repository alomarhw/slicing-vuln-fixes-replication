static RBinInfo *info(RBinFile *arch) {
	struct r_bin_bflt_obj *obj = NULL; 
	RBinInfo *info = NULL; 
	if (!arch || !arch->o || !arch->o->bin_obj) {
		return NULL;
	}
	obj = (struct r_bin_bflt_obj*)arch->o->bin_obj;
	if (!(info = R_NEW0 (RBinInfo))) {
		return NULL;
	}
	info->file = arch->file ? strdup (arch->file) : NULL;
	info->rclass = strdup ("bflt");
	info->bclass = strdup ("bflt" );
	info->type = strdup ("bFLT (Executable file)");
	info->os = strdup ("Linux");
	info->subsystem = strdup ("Linux");
	info->arch = strdup ("arm");
	info->big_endian = obj->endian;
	info->bits = 32;
	info->has_va = false;
	info->dbg_info = 0;
	info->machine = strdup ("unknown");
	return info;
}

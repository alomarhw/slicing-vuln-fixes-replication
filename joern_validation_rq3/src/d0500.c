static void _print_strings(RCore *r, RList *list, int mode, int va) {
	int minstr = r_config_get_i (r->config, "bin.minstr");
	int maxstr = r_config_get_i (r->config, "bin.maxstr");
	RBin *bin = r->bin;
	RListIter *iter;
	RBinString *string;
	RBinSection *section;
	char *q;

	bin->minstrlen = minstr;
	bin->maxstrlen = maxstr;
	if (IS_MODE_JSON (mode)) {
		r_cons_printf ("[");
	}
	if (IS_MODE_RAD (mode)) {
		r_cons_printf ("fs strings");
	}
	if (IS_MODE_SET (mode) && r_config_get_i (r->config, "bin.strings")) {
		r_flag_space_set (r->flags, "strings");
		r_cons_break_push (NULL, NULL);
	}
	r_list_foreach (list, iter, string) {
		const char *section_name, *type_string;
		ut64 paddr, vaddr, addr;
		if (!string_filter (r, string->string)) {
			continue;
		}
		paddr = string->paddr;
		vaddr = r_bin_get_vaddr (bin, paddr, string->vaddr);
		addr = va ? vaddr : paddr;
		if (string->length < minstr) {
			continue;
		}
		if (maxstr && string->length > maxstr) {
			continue;
		}

		section = r_bin_get_section_at (r_bin_cur_object (bin), paddr, 0);
		section_name = section ? section->name : "unknown";
		type_string = r_bin_string_type (string->type);
		if (IS_MODE_SET (mode)) {
			char *f_name, *str;
			if (r_cons_is_breaked ()) {
				break;
			}
			r_meta_add (r->anal, R_META_TYPE_STRING, addr, addr + string->size, string->string);
			f_name = strdup (string->string);
			r_name_filter (f_name, -1);
			if (r->bin->prefix) {
				str = r_str_newf ("%s.str.%s", r->bin->prefix, f_name);
			} else {
				str = r_str_newf ("str.%s", f_name);
			}
			r_flag_set (r->flags, str, addr, string->size);
			free (str);
			free (f_name);
		} else if (IS_MODE_SIMPLE (mode)) {
			r_cons_printf ("0x%"PFMT64x" %d %d %s\n", addr, string->size, string->length, string->string);
		} else if (IS_MODE_SIMPLEST (mode)) {
			r_cons_println (string->string);
		} else if (IS_MODE_JSON (mode)) {
			q = r_base64_encode_dyn (string->string, -1);
			r_cons_printf ("%s{\"vaddr\":%"PFMT64d
				",\"paddr\":%"PFMT64d",\"ordinal\":%d"
				",\"size\":%d,\"length\":%d,\"section\":\"%s\","
				"\"type\":\"%s\",\"string\":\"%s\"}",
				iter->p ? ",": "",
				vaddr, paddr, string->ordinal, string->size,
				string->length, section_name, type_string, q);
			free (q);
		} else if (IS_MODE_RAD (mode)) {
			char *f_name, *str;
			f_name = strdup (string->string);
			r_name_filter (f_name, R_FLAG_NAME_SIZE);
			if (r->bin->prefix) {
				str = r_str_newf ("%s.str.%s", r->bin->prefix, f_name);
				r_cons_printf ("f %s.str.%s %"PFMT64d" @ 0x%08"PFMT64x"\n"
					"Cs %"PFMT64d" @ 0x%08"PFMT64x"\n",
					r->bin->prefix, f_name, string->size, addr,
					string->size, addr);
			} else {
				str = r_str_newf ("str.%s", f_name);
				r_cons_printf ("f str.%s %"PFMT64d" @ 0x%08"PFMT64x"\n"
					"Cs %"PFMT64d" @ 0x%08"PFMT64x"\n",
					f_name, string->size, addr,
					string->size, addr);
			}
			free (str);
			free (f_name);
		} else {
			r_cons_printf ("vaddr=0x%08"PFMT64x" paddr=0x%08"
				PFMT64x" ordinal=%03u sz=%u len=%u "
				"section=%s type=%s string=%s\n",
				vaddr, paddr, string->ordinal, string->size,
				string->length, section_name, type_string,
				string->string);
		}
	}
	if (IS_MODE_JSON (mode)) {
		r_cons_printf ("]");
	}
	if (IS_MODE_SET (mode)) {
		r_cons_break_pop ();
	}
}

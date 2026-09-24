struct r_bin_pe_addr_t *PE_(check_msvcseh) (struct PE_(r_bin_pe_obj_t) *bin) {
	struct r_bin_pe_addr_t* entry;
	ut8 b[512];
	int n = 0;
	if (!bin || !bin->b) {
		return 0LL;
	}
	entry = PE_(r_bin_pe_get_entrypoint) (bin);
	ZERO_FILL (b);
	if (r_buf_read_at (bin->b, entry->paddr, b, sizeof (b)) < 0) {
		bprintf ("Warning: Cannot read entry at 0x%08"PFMT64x "\n", entry->paddr);
		free (entry);
		return NULL;
	}
	if (b[0] == 0xe8 && b[5] == 0xe9) {
		const st32 jmp_dst = r_read_ble32 (b + 6, bin->big_endian);
		entry->paddr += (5 + 5 + jmp_dst);
		entry->vaddr += (5 + 5 + jmp_dst);
		if (r_buf_read_at (bin->b, entry->paddr, b, sizeof (b)) > 0) {
			ut32 imageBase = bin->nt_headers->optional_header.ImageBase;
			for (n = 0; n < sizeof (b) - 6; n++) {
				const ut32 tmp_imgbase = r_read_ble32 (b + n + 1, bin->big_endian);
				if (b[n] == 0x68 && tmp_imgbase == imageBase && b[n + 5] == 0xe8) {
					const st32 call_dst = r_read_ble32 (b + n + 6, bin->big_endian);
					entry->paddr += (n + 5 + 5 + call_dst);
					entry->vaddr += (n + 5 + 5 + call_dst);
					return entry;
				}
			}
			for (n = 0; n < sizeof (b) - 6; n++) {
				if (b[n] == 0x50 && b[n+1] == 0xff && b[n + 3] == 0xff && b[n + 5] == 0xe8) {
					const st32 call_dst = r_read_ble32 (b + n + 6, bin->big_endian);
					entry->paddr += (n + 5 + 5 + call_dst);
					entry->vaddr += (n + 5 + 5 + call_dst);
					return entry;
				}
			}
			for (n = 0; n < sizeof (b) - 20; n++) {
				if (b[n] == 0x50 && b[n + 1] == 0xff && b[n + 7] == 0xff && b[n + 13] == 0xe8) {
					const st32 call_dst = r_read_ble32 (b + n + 14, bin->big_endian);
					entry->paddr += (n + 5 + 13 + call_dst);
					entry->vaddr += (n + 5 + 13 + call_dst);
					return entry;
				}
			}
			for (n = 0; n < sizeof (b) - 5; n++) {
				if (b[n] == 0x50 && b[n + 1] == 0x57 && b[n + 2] == 0xff && b[n + 4] == 0xe8) {
					const st32 call_dst = r_read_ble32 (b + n + 5, bin->big_endian);
					entry->paddr += (n + 5 + 4 + call_dst);
					entry->vaddr += (n + 5 + 4 + call_dst);
					return entry;
				}
			}

		}
	}
	if (b[4] == 0xe8 && b[13] == 0xe9) {
		const st32 jmp_dst = r_read_ble32 (b + 14, bin->big_endian);
		entry->paddr += (5 + 13 + jmp_dst);
		entry->vaddr += (5 + 13 + jmp_dst);
		if (r_buf_read_at (bin->b, entry->paddr, b, sizeof (b)) > 0) {
			for (n = 0; n < sizeof (b) - 13; n++) {
				if (b[n] == 0x4c && b[n + 3] == 0x48 && b[n + 6] == 0x8b && b[n + 8] == 0xe8) {
					const st32 call_dst = r_read_ble32 (b + n + 9, bin->big_endian);
					entry->paddr += (n + 5 + 8 + call_dst);
					entry->vaddr += (n + 5 + 8 + call_dst);
					return entry;
				}
			}
		}
	}
	if (b[188] == 0x50 && b[201] == 0xe8) {
		const st32 call_dst = r_read_ble32 (b + 202, bin->big_endian);
		entry->paddr += (201 + 5 + call_dst);
		entry->vaddr += (201 + 5 + call_dst);
		return entry;
	}

	if (b[292] == 0x50 && b[303] == 0xe8) {
		const st32 call_dst = r_read_ble32 (b + 304, bin->big_endian);
		entry->paddr += (303 + 5 + call_dst);
		entry->vaddr += (303 + 5 + call_dst);
		return entry;
	}

	free (entry);
	return NULL;
}

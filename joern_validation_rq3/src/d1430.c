static int load_bitmap_header(struct bitmap_index *index)
{
	struct bitmap_disk_header *header = (void *)index->map;

	if (index->map_size < sizeof(*header) + 20)
		return error("Corrupted bitmap index (missing header data)");

	if (memcmp(header->magic, BITMAP_IDX_SIGNATURE, sizeof(BITMAP_IDX_SIGNATURE)) != 0)
		return error("Corrupted bitmap index file (wrong header)");

	index->version = ntohs(header->version);
	if (index->version != 1)
		return error("Unsupported version for bitmap index file (%d)", index->version);

	/* Parse known bitmap format options */
	{
		uint32_t flags = ntohs(header->options);

		if ((flags & BITMAP_OPT_FULL_DAG) == 0)
			return error("Unsupported options for bitmap index file "
				"(Git requires BITMAP_OPT_FULL_DAG)");

		if (flags & BITMAP_OPT_HASH_CACHE) {
			unsigned char *end = index->map + index->map_size - 20;
			index->hashes = ((uint32_t *)end) - index->pack->num_objects;
		}
	}

	index->entry_count = ntohl(header->entry_count);
	index->map_pos += sizeof(*header);
	return 0;
}

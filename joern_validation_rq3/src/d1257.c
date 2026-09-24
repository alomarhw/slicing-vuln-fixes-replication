static int ext4_ext_rm_idx(handle_t *handle, struct inode *inode,
			struct ext4_ext_path *path)
{
	int err;
	ext4_fsblk_t leaf;

	/* free index block */
	path--;
	leaf = idx_pblock(path->p_idx);
	BUG_ON(path->p_hdr->eh_entries == 0);
	err = ext4_ext_get_access(handle, inode, path);
	if (err)
		return err;
	le16_add_cpu(&path->p_hdr->eh_entries, -1);
	err = ext4_ext_dirty(handle, inode, path);
	if (err)
		return err;
	ext_debug("index is empty, remove it, free block %llu\n", leaf);
	ext4_free_blocks(handle, inode, 0, leaf, 1,
			 EXT4_FREE_BLOCKS_METADATA | EXT4_FREE_BLOCKS_FORGET);
	return err;
}

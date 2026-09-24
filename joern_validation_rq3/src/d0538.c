static void add_root_to_dirty_list(struct btrfs_root *root)
{
	spin_lock(&root->fs_info->trans_lock);
	if (test_bit(BTRFS_ROOT_TRACK_DIRTY, &root->state) &&
	    list_empty(&root->dirty_list)) {
		list_add(&root->dirty_list,
			 &root->fs_info->dirty_cowonly_roots);
	}
	spin_unlock(&root->fs_info->trans_lock);
}

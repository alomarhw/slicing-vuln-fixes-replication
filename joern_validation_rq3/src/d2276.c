struct inode *udf_iget(struct super_block *sb, struct kernel_lb_addr *ino)
{
	unsigned long block = udf_get_lb_pblock(sb, ino, 0);
	struct inode *inode = iget_locked(sb, block);

	if (!inode)
		return NULL;

	if (inode->i_state & I_NEW) {
		memcpy(&UDF_I(inode)->i_location, ino, sizeof(struct kernel_lb_addr));
		__udf_read_inode(inode);
		unlock_new_inode(inode);
	}

	if (is_bad_inode(inode))
		goto out_iput;

	if (ino->logicalBlockNum >= UDF_SB(sb)->
			s_partmaps[ino->partitionReferenceNum].s_partition_len) {
		udf_debug("block=%d, partition=%d out of range\n",
			  ino->logicalBlockNum, ino->partitionReferenceNum);
		make_bad_inode(inode);
		goto out_iput;
	}

	return inode;

 out_iput:
	iput(inode);
	return NULL;
}

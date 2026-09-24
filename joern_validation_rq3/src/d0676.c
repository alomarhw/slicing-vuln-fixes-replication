static int __btrfs_correct_data_nocsum(struct inode *inode,
				       struct btrfs_io_bio *io_bio)
{
	struct bio_vec *bvec;
	struct btrfs_retry_complete done;
	u64 start;
	int i;
	int ret;

	start = io_bio->logical;
	done.inode = inode;

	bio_for_each_segment_all(bvec, &io_bio->bio, i) {
try_again:
		done.uptodate = 0;
		done.start = start;
		init_completion(&done.done);

		ret = dio_read_error(inode, &io_bio->bio, bvec->bv_page, start,
				     start + bvec->bv_len - 1,
				     io_bio->mirror_num,
				     btrfs_retry_endio_nocsum, &done);
		if (ret)
			return ret;

		wait_for_completion(&done.done);

		if (!done.uptodate) {
			/* We might have another mirror, so try again */
			goto try_again;
		}

		start += bvec->bv_len;
	}

	return 0;
}

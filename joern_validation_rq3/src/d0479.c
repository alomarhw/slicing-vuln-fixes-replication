int write_block(int file_fd, char *buffer, int size, long long hole, int sparse)
{
	off_t off = hole;

	if(hole) {
		if(sparse && lseek_broken == FALSE) {
			 int error = lseek(file_fd, off, SEEK_CUR);
			 if(error == -1)
				/* failed to seek beyond end of file */
				lseek_broken = TRUE;
		}

		if((sparse == FALSE || lseek_broken) && zero_data == NULL) {
			if((zero_data = malloc(block_size)) == NULL)
				EXIT_UNSQUASH("write_block: failed to alloc "
					"zero data block\n");
			memset(zero_data, 0, block_size);
		}

		if(sparse == FALSE || lseek_broken) {
			int blocks = (hole + block_size -1) / block_size;
			int avail_bytes, i;
			for(i = 0; i < blocks; i++, hole -= avail_bytes) {
				avail_bytes = hole > block_size ? block_size :
					hole;
				if(write_bytes(file_fd, zero_data, avail_bytes)
						== -1)
					goto failure;
			}
		}
	}

	if(write_bytes(file_fd, buffer, size) == -1)
		goto failure;

	return TRUE;

failure:
	FAILED = TRUE;
	return FALSE;
}

void close_wake(int fd)
{
	close(fd);

	if (!open_unlimited) {
		pthread_mutex_lock(&open_mutex);
		open_count ++;
		pthread_cond_signal(&open_empty);
		pthread_mutex_unlock(&open_mutex);
	}
}

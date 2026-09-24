void zend_shared_alloc_save_state(void)
{
	int i;

	for (i = 0; i < ZSMMG(shared_segments_count); i++) {
		ZSMMG(shared_memory_state).positions[i] = ZSMMG(shared_segments)[i]->pos;
	}
	ZSMMG(shared_memory_state).shared_free = ZSMMG(shared_free);
}

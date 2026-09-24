int git_index__find_pos(
	size_t *out, git_index *index, const char *path, size_t path_len, int stage)
{
	assert(index && path);
	return index_find(out, index, path, path_len, stage);
}

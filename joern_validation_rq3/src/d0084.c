dump_splay(cmap_splay *tree, unsigned int node, int depth, const char *pre)
{
	int i;

	if (node == EMPTY)
		return;

	for (i = 0; i < depth; i++)
		fprintf(stderr, " ");
	fprintf(stderr, "%s%d:", pre, node);
	if (tree[node].parent == EMPTY)
		fprintf(stderr, "^EMPTY");
	else
		fprintf(stderr, "^%d", tree[node].parent);
	if (tree[node].left == EMPTY)
		fprintf(stderr, "<EMPTY");
	else
		fprintf(stderr, "<%d", tree[node].left);
	if (tree[node].right == EMPTY)
		fprintf(stderr, ">EMPTY");
	else
		fprintf(stderr, ">%d", tree[node].right);
	fprintf(stderr, "(%x,%x,%x,%d)\n", tree[node].low, tree[node].high, tree[node].out, tree[node].many);
	assert(tree[node].parent == EMPTY || depth);
	assert(tree[node].left == EMPTY || tree[tree[node].left].parent == node);
	assert(tree[node].right == EMPTY || tree[tree[node].right].parent == node);
	dump_splay(tree, tree[node].left, depth+1, "L");
	dump_splay(tree, tree[node].right, depth+1, "R");
}

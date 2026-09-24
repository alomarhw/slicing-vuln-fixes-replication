static struct sock *raw_get_next(struct seq_file *seq, struct sock *sk)
{
	struct raw_iter_state *state = raw_seq_private(seq);

	do {
		sk = sk_next(sk);
try_again:
		;
	} while (sk && sock_net(sk) != seq_file_net(seq));

	if (!sk && ++state->bucket < RAW_HTABLE_SIZE) {
		sk = sk_head(&state->h->ht[state->bucket]);
		goto try_again;
	}
	return sk;
}

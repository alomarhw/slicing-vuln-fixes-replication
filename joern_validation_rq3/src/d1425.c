int __init client_init_data(void)
{
	/* zap out the client table */
	memset(&clienttablock, 0, sizeof(clienttablock));
	memset(&clienttab, 0, sizeof(clienttab));
	return 0;
}

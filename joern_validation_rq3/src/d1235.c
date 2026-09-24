static void hash_release(void *private)
{
	crypto_free_ahash(private);
}

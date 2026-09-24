static void __exit fini(void)
{
	crypto_unregister_shash(&sha512_alg);
	crypto_unregister_shash(&sha384_alg);
}

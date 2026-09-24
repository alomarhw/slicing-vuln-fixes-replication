static void __exit crypto_module_exit(void)
{
	crypto_unregister_template(&crypto_tmpl);
}

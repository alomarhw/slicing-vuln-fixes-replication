int crypto_ahash_setkey(struct crypto_ahash *tfm, const u8 *key,
			unsigned int keylen)
{
	unsigned long alignmask = crypto_ahash_alignmask(tfm);

	if ((unsigned long)key & alignmask)
		return ahash_setkey_unaligned(tfm, key, keylen);

	return tfm->setkey(tfm, key, keylen);
}

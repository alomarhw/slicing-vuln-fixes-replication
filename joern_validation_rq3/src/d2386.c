static void __munlock_isolated_page(struct page *page)
{
	int ret = SWAP_AGAIN;

	/*
	 * Optimization: if the page was mapped just once, that's our mapping
	 * and we don't need to check all the other vmas.
	 */
	if (page_mapcount(page) > 1)
		ret = try_to_munlock(page);

	/* Did try_to_unlock() succeed or punt? */
	if (ret != SWAP_MLOCK)
		count_vm_event(UNEVICTABLE_PGMUNLOCKED);

	putback_lru_page(page);
}

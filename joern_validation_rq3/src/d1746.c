static void drain_alien_cache(struct kmem_cache *cachep,
				struct alien_cache **alien)
{
	int i = 0;
	struct alien_cache *alc;
	struct array_cache *ac;
	unsigned long flags;

	for_each_online_node(i) {
		alc = alien[i];
		if (alc) {
			LIST_HEAD(list);

			ac = &alc->ac;
			spin_lock_irqsave(&alc->lock, flags);
			__drain_alien_cache(cachep, ac, i, &list);
			spin_unlock_irqrestore(&alc->lock, flags);
			slabs_destroy(cachep, &list);
		}
	}
}

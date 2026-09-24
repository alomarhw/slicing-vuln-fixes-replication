MenuCacheNotifyId menu_cache_add_reload_notify(MenuCache* cache, MenuCacheReloadNotify func, gpointer user_data)
{
    GSList* l = g_slist_alloc();
    CacheReloadNotifier* n = g_slice_new(CacheReloadNotifier);
    gboolean is_first;
    n->func = func;
    n->user_data = user_data;
    l->data = n;
    MENU_CACHE_LOCK;
    is_first = (cache->root_dir == NULL && cache->notifiers == NULL);
    cache->notifiers = g_slist_concat( cache->notifiers, l );
    /* reload existing file first so it will be ready right away */
    if(is_first && cache->reload_id == 0)
        cache->reload_id = g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                                           menu_cache_reload_idle,
                                           menu_cache_ref(cache),
                                           (GDestroyNotify)menu_cache_unref);
    MENU_CACHE_UNLOCK;
    return (MenuCacheNotifyId)l;
}

    void free_descriptor_buffer(struct b43_dmaring *ring,
				struct b43_dmadesc_meta *meta)
{
	if (meta->skb) {
		dev_kfree_skb_any(meta->skb);
		meta->skb = NULL;
	}
}

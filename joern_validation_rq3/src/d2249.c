static int __init arm64_dma_init(void)
{
	int ret;

	dma_ops = &swiotlb_dma_ops;

	ret = atomic_pool_init();

	return ret;
}

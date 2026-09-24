static struct bsg_command *bsg_alloc_command(struct bsg_device *bd)
{
	struct bsg_command *bc = ERR_PTR(-EINVAL);

	spin_lock_irq(&bd->lock);

	if (bd->queued_cmds >= bd->max_queue)
		goto out;

	bd->queued_cmds++;
	spin_unlock_irq(&bd->lock);

	bc = kmem_cache_zalloc(bsg_cmd_cachep, GFP_KERNEL);
	if (unlikely(!bc)) {
		spin_lock_irq(&bd->lock);
		bd->queued_cmds--;
		bc = ERR_PTR(-ENOMEM);
		goto out;
	}

	bc->bd = bd;
	INIT_LIST_HEAD(&bc->list);
	dprintk("%s: returning free cmd %p\n", bd->name, bc);
	return bc;
out:
	spin_unlock_irq(&bd->lock);
	return bc;
}

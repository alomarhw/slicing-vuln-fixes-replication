static void nested_free_all_saved_vmcss(struct vcpu_vmx *vmx)
{
	struct vmcs02_list *item, *n;

	WARN_ON(vmx->loaded_vmcs != &vmx->vmcs01);
	list_for_each_entry_safe(item, n, &vmx->nested.vmcs02_pool, list) {
		/*
		 * Something will leak if the above WARN triggers.  Better than
		 * a use-after-free.
		 */
		if (vmx->loaded_vmcs == &item->vmcs02)
			continue;

		free_loaded_vmcs(&item->vmcs02);
		list_del(&item->list);
		kfree(item);
		vmx->nested.vmcs02_num--;
	}
}

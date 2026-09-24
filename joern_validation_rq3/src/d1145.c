int security_bounded_transition(u32 old_sid, u32 new_sid)
{
	struct context *old_context, *new_context;
	struct type_datum *type;
	int index;
	int rc;

	read_lock(&policy_rwlock);

	rc = -EINVAL;
	old_context = sidtab_search(&sidtab, old_sid);
	if (!old_context) {
		printk(KERN_ERR "SELinux: %s: unrecognized SID %u\n",
		       __func__, old_sid);
		goto out;
	}

	rc = -EINVAL;
	new_context = sidtab_search(&sidtab, new_sid);
	if (!new_context) {
		printk(KERN_ERR "SELinux: %s: unrecognized SID %u\n",
		       __func__, new_sid);
		goto out;
	}

	rc = 0;
	/* type/domain unchanged */
	if (old_context->type == new_context->type)
		goto out;

	index = new_context->type;
	while (true) {
		type = flex_array_get_ptr(policydb.type_val_to_struct_array,
					  index - 1);
		BUG_ON(!type);

		/* not bounded anymore */
		rc = -EPERM;
		if (!type->bounds)
			break;

		/* @newsid is bounded by @oldsid */
		rc = 0;
		if (type->bounds == old_context->type)
			break;

		index = type->bounds;
	}

	if (rc) {
		char *old_name = NULL;
		char *new_name = NULL;
		u32 length;

		if (!context_struct_to_string(old_context,
					      &old_name, &length) &&
		    !context_struct_to_string(new_context,
					      &new_name, &length)) {
			audit_log(current->audit_context,
				  GFP_ATOMIC, AUDIT_SELINUX_ERR,
				  "op=security_bounded_transition "
				  "result=denied "
				  "oldcontext=%s newcontext=%s",
				  old_name, new_name);
		}
		kfree(new_name);
		kfree(old_name);
	}
out:
	read_unlock(&policy_rwlock);

	return rc;
}

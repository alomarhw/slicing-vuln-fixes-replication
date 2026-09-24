void __audit_inode_child(struct inode *parent,
			 const struct dentry *dentry,
			 const unsigned char type)
{
	struct audit_context *context = current->audit_context;
	struct inode *inode = d_backing_inode(dentry);
	const char *dname = dentry->d_name.name;
	struct audit_names *n, *found_parent = NULL, *found_child = NULL;

	if (!context->in_syscall)
		return;

	if (inode)
		handle_one(inode);

	/* look for a parent entry first */
	list_for_each_entry(n, &context->names_list, list) {
		if (!n->name ||
		    (n->type != AUDIT_TYPE_PARENT &&
		     n->type != AUDIT_TYPE_UNKNOWN))
			continue;

		if (n->ino == parent->i_ino && n->dev == parent->i_sb->s_dev &&
		    !audit_compare_dname_path(dname,
					      n->name->name, n->name_len)) {
			if (n->type == AUDIT_TYPE_UNKNOWN)
				n->type = AUDIT_TYPE_PARENT;
			found_parent = n;
			break;
		}
	}

	/* is there a matching child entry? */
	list_for_each_entry(n, &context->names_list, list) {
		/* can only match entries that have a name */
		if (!n->name ||
		    (n->type != type && n->type != AUDIT_TYPE_UNKNOWN))
			continue;

		if (!strcmp(dname, n->name->name) ||
		    !audit_compare_dname_path(dname, n->name->name,
						found_parent ?
						found_parent->name_len :
						AUDIT_NAME_FULL)) {
			if (n->type == AUDIT_TYPE_UNKNOWN)
				n->type = type;
			found_child = n;
			break;
		}
	}

	if (!found_parent) {
		/* create a new, "anonymous" parent record */
		n = audit_alloc_name(context, AUDIT_TYPE_PARENT);
		if (!n)
			return;
		audit_copy_inode(n, NULL, parent);
	}

	if (!found_child) {
		found_child = audit_alloc_name(context, type);
		if (!found_child)
			return;

		/* Re-use the name belonging to the slot for a matching parent
		 * directory. All names for this context are relinquished in
		 * audit_free_names() */
		if (found_parent) {
			found_child->name = found_parent->name;
			found_child->name_len = AUDIT_NAME_FULL;
			found_child->name->refcnt++;
		}
	}

	if (inode)
		audit_copy_inode(found_child, dentry, inode);
	else
		found_child->ino = AUDIT_INO_UNSET;
}

cifs_acl_to_fattr(struct cifs_sb_info *cifs_sb, struct cifs_fattr *fattr,
		  struct inode *inode, const char *path,
		  const struct cifs_fid *pfid)
{
	struct cifs_ntsd *pntsd = NULL;
	u32 acllen = 0;
	int rc = 0;
	struct tcon_link *tlink = cifs_sb_tlink(cifs_sb);
	struct cifs_tcon *tcon;

	cifs_dbg(NOISY, "converting ACL to mode for %s\n", path);

	if (IS_ERR(tlink))
		return PTR_ERR(tlink);
	tcon = tlink_tcon(tlink);

	if (pfid && (tcon->ses->server->ops->get_acl_by_fid))
		pntsd = tcon->ses->server->ops->get_acl_by_fid(cifs_sb, pfid,
							  &acllen);
	else if (tcon->ses->server->ops->get_acl)
		pntsd = tcon->ses->server->ops->get_acl(cifs_sb, inode, path,
							&acllen);
	else {
		cifs_put_tlink(tlink);
		return -EOPNOTSUPP;
	}
	/* if we can retrieve the ACL, now parse Access Control Entries, ACEs */
	if (IS_ERR(pntsd)) {
		rc = PTR_ERR(pntsd);
		cifs_dbg(VFS, "%s: error %d getting sec desc\n", __func__, rc);
	} else {
		rc = parse_sec_desc(cifs_sb, pntsd, acllen, fattr);
		kfree(pntsd);
		if (rc)
			cifs_dbg(VFS, "parse sec desc failed rc = %d\n", rc);
	}

	cifs_put_tlink(tlink);

	return rc;
}

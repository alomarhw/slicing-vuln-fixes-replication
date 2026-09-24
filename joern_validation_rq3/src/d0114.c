static int set_qf_name(struct super_block *sb, int qtype, substring_t *args)
{
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	char *qname;

	if (sb_any_quota_loaded(sb) &&
		!sbi->s_qf_names[qtype]) {
		ext4_msg(sb, KERN_ERR,
			"Cannot change journaled "
			"quota options when quota turned on");
		return 0;
	}
	qname = match_strdup(args);
	if (!qname) {
		ext4_msg(sb, KERN_ERR,
			"Not enough memory for storing quotafile name");
		return 0;
	}
	if (sbi->s_qf_names[qtype] &&
		strcmp(sbi->s_qf_names[qtype], qname)) {
		ext4_msg(sb, KERN_ERR,
			"%s quota file already specified", QTYPE2NAME(qtype));
		kfree(qname);
		return 0;
	}
	sbi->s_qf_names[qtype] = qname;
	if (strchr(sbi->s_qf_names[qtype], '/')) {
		ext4_msg(sb, KERN_ERR,
			"quotafile must be on filesystem root");
		kfree(sbi->s_qf_names[qtype]);
		sbi->s_qf_names[qtype] = NULL;
		return 0;
	}
	set_opt(sb, QUOTA);
	return 1;
}

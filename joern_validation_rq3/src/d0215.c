static int ebt_compat_entry_padsize(void)
{
	BUILD_BUG_ON(XT_ALIGN(sizeof(struct ebt_entry_match)) <
			COMPAT_XT_ALIGN(sizeof(struct compat_ebt_entry_mwt)));
	return (int) XT_ALIGN(sizeof(struct ebt_entry_match)) -
			COMPAT_XT_ALIGN(sizeof(struct compat_ebt_entry_mwt));
}

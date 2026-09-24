static int iscsi_check_string_or_list_value(struct iscsi_param *param, char *value)
{
	if (IS_PSTATE_PROPOSER(param))
		return 0;

	if (IS_TYPERANGE_AUTH_PARAM(param)) {
		if (strcmp(value, KRB5) && strcmp(value, SPKM1) &&
		    strcmp(value, SPKM2) && strcmp(value, SRP) &&
		    strcmp(value, CHAP) && strcmp(value, NONE)) {
			pr_err("Illegal value for \"%s\", must be"
				" \"%s\", \"%s\", \"%s\", \"%s\", \"%s\""
				" or \"%s\".\n", param->name, KRB5,
					SPKM1, SPKM2, SRP, CHAP, NONE);
			return -1;
		}
	}
	if (IS_TYPERANGE_DIGEST_PARAM(param)) {
		if (strcmp(value, CRC32C) && strcmp(value, NONE)) {
			pr_err("Illegal value for \"%s\", must be"
				" \"%s\" or \"%s\".\n", param->name,
					CRC32C, NONE);
			return -1;
		}
	}
	if (IS_TYPERANGE_SESSIONTYPE(param)) {
		if (strcmp(value, DISCOVERY) && strcmp(value, NORMAL)) {
			pr_err("Illegal value for \"%s\", must be"
				" \"%s\" or \"%s\".\n", param->name,
					DISCOVERY, NORMAL);
			return -1;
		}
	}

	return 0;
}

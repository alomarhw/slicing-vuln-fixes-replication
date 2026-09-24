static bool decode_extended_dn_request(void *mem_ctx, DATA_BLOB in, void *_out)
{
	void **out = (void **)_out;
	struct asn1_data *data;
	struct ldb_extended_dn_control *ledc;

	/* The content of this control is optional */
	if (in.length == 0) {
		*out = NULL;
		return true;
	}

	data = asn1_init(mem_ctx);
	if (!data) return false;

	if (!asn1_load(data, in)) {
		return false;
	}

	ledc = talloc(mem_ctx, struct ldb_extended_dn_control);
	if (!ledc) {
		return false;
	}

	if (!asn1_start_tag(data, ASN1_SEQUENCE(0))) {
		return false;
	}

	if (!asn1_read_Integer(data, &(ledc->type))) {
		return false;
	}
	
	if (!asn1_end_tag(data)) {
		return false;
	}

	*out = ledc;

	return true;
}

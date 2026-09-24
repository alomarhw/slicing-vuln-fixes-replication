static int encap_same(const struct bgp_attr_encap_subtlv *h1,
		      const struct bgp_attr_encap_subtlv *h2)
{
	const struct bgp_attr_encap_subtlv *p;
	const struct bgp_attr_encap_subtlv *q;

	if (h1 == h2)
		return 1;
	if (h1 == NULL || h2 == NULL)
		return 0;

	for (p = h1; p; p = p->next) {
		for (q = h2; q; q = q->next) {
			if ((p->type == q->type) && (p->length == q->length)
			    && !memcmp(p->value, q->value, p->length)) {

				break;
			}
		}
		if (!q)
			return 0;
	}

	for (p = h2; p; p = p->next) {
		for (q = h1; q; q = q->next) {
			if ((p->type == q->type) && (p->length == q->length)
			    && !memcmp(p->value, q->value, p->length)) {

				break;
			}
		}
		if (!q)
			return 0;
	}

	return 1;
}

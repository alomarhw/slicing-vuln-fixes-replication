encode_POP_MPLS(const struct ofpact_pop_mpls *pop_mpls,
                enum ofp_version ofp_version, struct ofpbuf *out)
{
    put_OFPAT_POP_MPLS(out, ofp_version, pop_mpls->ethertype);
}

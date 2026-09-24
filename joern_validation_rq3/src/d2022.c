ofputil_put_ofp11_table_stats(const struct ofputil_table_stats *stats,
                              const struct ofputil_table_features *features,
                              struct ofpbuf *buf)
{
    struct mf_bitmap wc = wild_or_nonmatchable_fields(features);
    struct ofp11_table_stats *out;

    out = ofpbuf_put_zeros(buf, sizeof *out);
    out->table_id = features->table_id;
    ovs_strlcpy(out->name, features->name, sizeof out->name);
    out->wildcards = mf_bitmap_to_of11(&wc);
    out->match = mf_bitmap_to_of11(&features->match);
    out->instructions = ovsinst_bitmap_to_openflow(
        features->nonmiss.instructions, OFP11_VERSION);
    out->write_actions = ofpact_bitmap_to_openflow(
        features->nonmiss.write.ofpacts, OFP11_VERSION);
    out->apply_actions = ofpact_bitmap_to_openflow(
        features->nonmiss.apply.ofpacts, OFP11_VERSION);
    out->config = htonl(features->miss_config);
    out->max_entries = htonl(features->max_entries);
    out->active_count = htonl(stats->active_count);
    out->lookup_count = htonll(stats->lookup_count);
    out->matched_count = htonll(stats->matched_count);
}

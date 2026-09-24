add_conjunction(struct ofpbuf *out,
                uint32_t id, uint8_t clause, uint8_t n_clauses)
{
    struct ofpact_conjunction *oc;

    oc = ofpact_put_CONJUNCTION(out);
    oc->id = id;
    oc->clause = clause;
    oc->n_clauses = n_clauses;
}

parse_SET_MPLS_TTL(char *arg, struct ofpbuf *ofpacts,
                   enum ofputil_protocol *usable_protocols OVS_UNUSED)
{
    struct ofpact_mpls_ttl *mpls_ttl = ofpact_put_SET_MPLS_TTL(ofpacts);

    if (*arg == '\0') {
        return xstrdup("set_mpls_ttl: expected ttl.");
    }

    mpls_ttl->ttl = atoi(arg);
    return NULL;
}

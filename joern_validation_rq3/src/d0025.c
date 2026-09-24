ofputil_frag_handling_from_string(const char *s,
                                  enum ofputil_frag_handling *frag)
{
    if (!strcasecmp(s, "normal")) {
        *frag = OFPUTIL_FRAG_NORMAL;
    } else if (!strcasecmp(s, "drop")) {
        *frag = OFPUTIL_FRAG_DROP;
    } else if (!strcasecmp(s, "reassemble")) {
        *frag = OFPUTIL_FRAG_REASM;
    } else if (!strcasecmp(s, "nx-match")) {
        *frag = OFPUTIL_FRAG_NX_MATCH;
    } else {
        return false;
    }
    return true;
}

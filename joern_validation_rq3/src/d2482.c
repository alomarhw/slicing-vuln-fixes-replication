ofproto_get_stp_status(struct ofproto *ofproto,
                       struct ofproto_stp_status *s)
{
    return (ofproto->ofproto_class->get_stp_status
            ? ofproto->ofproto_class->get_stp_status(ofproto, s)
            : EOPNOTSUPP);
}

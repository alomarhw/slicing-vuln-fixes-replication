static void unix_get_secdata(struct scm_cookie *scm, struct sk_buff *skb)
{
	memcpy(UNIXSID(skb), &scm->secid, sizeof(u32));
}

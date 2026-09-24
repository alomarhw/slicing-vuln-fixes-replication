static int SRP_user_pwd_set_sv(SRP_user_pwd *vinfo, const char *s,
                               const char *v)
{
    unsigned char tmp[MAX_LEN];
    int len;

    if (strlen(s) > MAX_LEN || strlen(v) > MAX_LEN)
        return 0;
    len = t_fromb64(tmp, v);
    if (NULL == (vinfo->v = BN_bin2bn(tmp, len, NULL)))
        return 0;
    len = t_fromb64(tmp, s);
    return ((vinfo->s = BN_bin2bn(tmp, len, NULL)) != NULL);
}

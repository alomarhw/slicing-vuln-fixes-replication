virDomainMigrateVersion3(virDomainPtr domain,
                         virConnectPtr dconn,
                         const char *xmlin,
                         unsigned long flags,
                         const char *dname,
                         const char *uri,
                         unsigned long bandwidth)
{
    return virDomainMigrateVersion3Full(domain, dconn, xmlin, dname, uri,
                                        bandwidth, NULL, 0, false, flags);
}

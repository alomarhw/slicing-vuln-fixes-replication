size_t mptsas_config_ioc_1(MPTSASState *s, uint8_t **data, int address)
{
    return MPTSAS_CONFIG_PACK(1, MPI_CONFIG_PAGETYPE_IOC, 0x03,
                              "*l*l*b*b*b*b");
}

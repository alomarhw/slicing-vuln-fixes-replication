static inline int ohci_put_td(OHCIState *ohci,
                              dma_addr_t addr, struct ohci_td *td)
{
    return put_dwords(ohci, addr, (uint32_t *)td, sizeof(*td) >> 2);
}

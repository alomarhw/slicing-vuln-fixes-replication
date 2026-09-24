static inline bool ehci_enabled(EHCIState *s)
{
    return s->usbcmd & USBCMD_RUNSTOP;
}

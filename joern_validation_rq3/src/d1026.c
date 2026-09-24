static EHCIQueue *ehci_state_fetchqh(EHCIState *ehci, int async)
{
    uint32_t entry;
    EHCIQueue *q;
    EHCIqh qh;

    entry = ehci_get_fetch_addr(ehci, async);
    q = ehci_find_queue_by_qh(ehci, entry, async);
    if (q == NULL) {
        q = ehci_alloc_queue(ehci, entry, async);
    }

    q->seen++;
    if (q->seen > 1) {
        /* we are going in circles -- stop processing */
        ehci_set_state(ehci, async, EST_ACTIVE);
        q = NULL;
        goto out;
    }

    if (get_dwords(ehci, NLPTR_GET(q->qhaddr),
                   (uint32_t *) &qh, sizeof(EHCIqh) >> 2) < 0) {
        q = NULL;
        goto out;
    }
    ehci_trace_qh(q, NLPTR_GET(q->qhaddr), &qh);

    /*
     * The overlay area of the qh should never be changed by the guest,
     * except when idle, in which case the reset is a nop.
     */
    if (!ehci_verify_qh(q, &qh)) {
        if (ehci_reset_queue(q) > 0) {
            ehci_trace_guest_bug(ehci, "guest updated active QH");
        }
    }
    q->qh = qh;

    q->transact_ctr = get_field(q->qh.epcap, QH_EPCAP_MULT);
    if (q->transact_ctr == 0) { /* Guest bug in some versions of windows */
        q->transact_ctr = 4;
    }

    if (q->dev == NULL) {
        q->dev = ehci_find_device(q->ehci,
                                  get_field(q->qh.epchar, QH_EPCHAR_DEVADDR));
    }

    if (async && (q->qh.epchar & QH_EPCHAR_H)) {

        /*  EHCI spec version 1.0 Section 4.8.3 & 4.10.1 */
        if (ehci->usbsts & USBSTS_REC) {
            ehci_clear_usbsts(ehci, USBSTS_REC);
        } else {
            DPRINTF("FETCHQH:  QH 0x%08x. H-bit set, reclamation status reset"
                       " - done processing\n", q->qhaddr);
            ehci_set_state(ehci, async, EST_ACTIVE);
            q = NULL;
            goto out;
        }
    }

#if EHCI_DEBUG
    if (q->qhaddr != q->qh.next) {
    DPRINTF("FETCHQH:  QH 0x%08x (h %x halt %x active %x) next 0x%08x\n",
               q->qhaddr,
               q->qh.epchar & QH_EPCHAR_H,
               q->qh.token & QTD_TOKEN_HALT,
               q->qh.token & QTD_TOKEN_ACTIVE,
               q->qh.next);
    }
#endif

    if (q->qh.token & QTD_TOKEN_HALT) {
        ehci_set_state(ehci, async, EST_HORIZONTALQH);

    } else if ((q->qh.token & QTD_TOKEN_ACTIVE) &&
               (NLPTR_TBIT(q->qh.current_qtd) == 0)) {
        q->qtdaddr = q->qh.current_qtd;
        ehci_set_state(ehci, async, EST_FETCHQTD);

    } else {
        /*  EHCI spec version 1.0 Section 4.10.2 */
        ehci_set_state(ehci, async, EST_ADVANCEQUEUE);
    }

out:
    return q;
}

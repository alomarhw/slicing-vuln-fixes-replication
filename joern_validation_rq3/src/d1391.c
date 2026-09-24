vmxnet3_setup_tx_offloads(VMXNET3State *s)
{
    switch (s->offload_mode) {
    case VMXNET3_OM_NONE:
        net_tx_pkt_build_vheader(s->tx_pkt, false, false, 0);
        break;

    case VMXNET3_OM_CSUM:
        net_tx_pkt_build_vheader(s->tx_pkt, false, true, 0);
        VMW_PKPRN("L4 CSO requested\n");
        break;

    case VMXNET3_OM_TSO:
        net_tx_pkt_build_vheader(s->tx_pkt, true, true,
            s->cso_or_gso_size);
        net_tx_pkt_update_ip_checksums(s->tx_pkt);
        VMW_PKPRN("GSO offload requested.");
        break;

    default:
        g_assert_not_reached();
        return false;
    }

    return true;
}

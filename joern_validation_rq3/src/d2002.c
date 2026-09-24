e1000e_core_reset(E1000ECore *core)
{
    int i;

    timer_del(core->autoneg_timer);

    e1000e_intrmgr_reset(core);

    memset(core->phy, 0, sizeof core->phy);
    memmove(core->phy, e1000e_phy_reg_init, sizeof e1000e_phy_reg_init);
    memset(core->mac, 0, sizeof core->mac);
    memmove(core->mac, e1000e_mac_reg_init, sizeof e1000e_mac_reg_init);

    core->rxbuf_min_shift = 1 + E1000_RING_DESC_LEN_SHIFT;

    if (qemu_get_queue(core->owner_nic)->link_down) {
        e1000e_link_down(core);
    }

    e1000x_reset_mac_addr(core->owner_nic, core->mac, core->permanent_mac);

    for (i = 0; i < ARRAY_SIZE(core->tx); i++) {
        net_tx_pkt_reset(core->tx[i].tx_pkt);
        memset(&core->tx[i].props, 0, sizeof(core->tx[i].props));
        core->tx[i].skip_cp = false;
    }
}

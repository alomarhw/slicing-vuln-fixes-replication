static void hns_rcb_ring_init(struct ring_pair_cb *ring_pair, int ring_type)
{
	struct hnae_queue *q = &ring_pair->q;
	struct hnae_ring *ring =
		(ring_type == RX_RING) ? &q->rx_ring : &q->tx_ring;
	dma_addr_t dma = ring->desc_dma_addr;

	if (ring_type == RX_RING) {
		dsaf_write_dev(q, RCB_RING_RX_RING_BASEADDR_L_REG,
			       (u32)dma);
		dsaf_write_dev(q, RCB_RING_RX_RING_BASEADDR_H_REG,
			       (u32)((dma >> 31) >> 1));

		hns_rcb_set_rx_ring_bs(q, ring->buf_size);

		dsaf_write_dev(q, RCB_RING_RX_RING_BD_NUM_REG,
			       ring_pair->port_id_in_comm);
		dsaf_write_dev(q, RCB_RING_RX_RING_PKTLINE_REG,
			       ring_pair->port_id_in_comm);
	} else {
		dsaf_write_dev(q, RCB_RING_TX_RING_BASEADDR_L_REG,
			       (u32)dma);
		dsaf_write_dev(q, RCB_RING_TX_RING_BASEADDR_H_REG,
			       (u32)((dma >> 31) >> 1));

		hns_rcb_set_tx_ring_bs(q, ring->buf_size);

		dsaf_write_dev(q, RCB_RING_TX_RING_BD_NUM_REG,
			       ring_pair->port_id_in_comm);
		dsaf_write_dev(q, RCB_RING_TX_RING_PKTLINE_REG,
			ring_pair->port_id_in_comm + HNS_RCB_TX_PKTLINE_OFFSET);
	}
}

static irqreturn_t tg3_msi_1shot(int irq, void *dev_id)
{
	struct tg3_napi *tnapi = dev_id;
	struct tg3 *tp = tnapi->tp;

	prefetch(tnapi->hw_status);
	if (tnapi->rx_rcb)
		prefetch(&tnapi->rx_rcb[tnapi->rx_rcb_ptr]);

	if (likely(!tg3_irq_sync(tp)))
		napi_schedule(&tnapi->napi);

	return IRQ_HANDLED;
}

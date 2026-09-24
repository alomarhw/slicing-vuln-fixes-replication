static void catc_ctrl_done(struct urb *urb)
{
	struct catc *catc = urb->context;
	struct ctrl_queue *q;
	unsigned long flags;
	int status = urb->status;

	if (status)
		dev_dbg(&urb->dev->dev, "ctrl_done, status %d, len %d.\n",
			status, urb->actual_length);

	spin_lock_irqsave(&catc->ctrl_lock, flags);

	q = catc->ctrl_queue + catc->ctrl_tail;

	if (q->dir) {
		if (q->buf && q->len)
			memcpy(q->buf, catc->ctrl_buf, q->len);
		else
			q->buf = catc->ctrl_buf;
	}

	if (q->callback)
		q->callback(catc, q);

	catc->ctrl_tail = (catc->ctrl_tail + 1) & (CTRL_QUEUE - 1);

	if (catc->ctrl_head != catc->ctrl_tail)
		catc_ctrl_run(catc);
	else
		clear_bit(CTRL_RUNNING, &catc->flags);

	spin_unlock_irqrestore(&catc->ctrl_lock, flags);
}

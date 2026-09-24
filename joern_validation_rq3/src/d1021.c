static void queue_interrupt(struct fuse_iqueue *fiq, struct fuse_req *req)
{
	spin_lock(&fiq->waitq.lock);
	if (test_bit(FR_FINISHED, &req->flags)) {
		spin_unlock(&fiq->waitq.lock);
		return;
	}
	if (list_empty(&req->intr_entry)) {
		list_add_tail(&req->intr_entry, &fiq->interrupts);
		wake_up_locked(&fiq->waitq);
	}
	spin_unlock(&fiq->waitq.lock);
	kill_fasync(&fiq->fasync, SIGIO, POLL_IN);
}

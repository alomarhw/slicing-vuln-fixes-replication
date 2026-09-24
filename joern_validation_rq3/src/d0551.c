ppp_input(struct ppp_channel *chan, struct sk_buff *skb)
{
	struct channel *pch = chan->ppp;
	int proto;

	if (!pch) {
		kfree_skb(skb);
		return;
	}

	read_lock_bh(&pch->upl);
	if (!pskb_may_pull(skb, 2)) {
		kfree_skb(skb);
		if (pch->ppp) {
			++pch->ppp->dev->stats.rx_length_errors;
			ppp_receive_error(pch->ppp);
		}
		goto done;
	}

	proto = PPP_PROTO(skb);
	if (!pch->ppp || proto >= 0xc000 || proto == PPP_CCPFRAG) {
		/* put it on the channel queue */
		skb_queue_tail(&pch->file.rq, skb);
		/* drop old frames if queue too long */
		while (pch->file.rq.qlen > PPP_MAX_RQLEN &&
		       (skb = skb_dequeue(&pch->file.rq)))
			kfree_skb(skb);
		wake_up_interruptible(&pch->file.rwait);
	} else {
		ppp_do_recv(pch->ppp, skb, pch);
	}

done:
	read_unlock_bh(&pch->upl);
}

static void iucv_callback_txdone(struct iucv_path *path,
				 struct iucv_message *msg)
{
	struct sock *sk = path->private;
	struct sk_buff *this = NULL;
	struct sk_buff_head *list = &iucv_sk(sk)->send_skb_q;
	struct sk_buff *list_skb = list->next;
	unsigned long flags;

	bh_lock_sock(sk);
	if (!skb_queue_empty(list)) {
		spin_lock_irqsave(&list->lock, flags);

		while (list_skb != (struct sk_buff *)list) {
			if (msg->tag != IUCV_SKB_CB(list_skb)->tag) {
				this = list_skb;
				break;
			}
			list_skb = list_skb->next;
		}
		if (this)
			__skb_unlink(this, list);

		spin_unlock_irqrestore(&list->lock, flags);

		if (this) {
			kfree_skb(this);
			/* wake up any process waiting for sending */
			iucv_sock_wake_msglim(sk);
		}
	}

	if (sk->sk_state == IUCV_CLOSING) {
		if (skb_queue_empty(&iucv_sk(sk)->send_skb_q)) {
			sk->sk_state = IUCV_CLOSED;
			sk->sk_state_change(sk);
		}
	}
	bh_unlock_sock(sk);

}

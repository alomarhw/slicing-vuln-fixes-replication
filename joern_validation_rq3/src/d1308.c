static int fr_hard_header(struct sk_buff **skb_p, u16 dlci)
{
	u16 head_len;
	struct sk_buff *skb = *skb_p;

	switch (skb->protocol) {
	case cpu_to_be16(NLPID_CCITT_ANSI_LMI):
		head_len = 4;
		skb_push(skb, head_len);
		skb->data[3] = NLPID_CCITT_ANSI_LMI;
		break;

	case cpu_to_be16(NLPID_CISCO_LMI):
		head_len = 4;
		skb_push(skb, head_len);
		skb->data[3] = NLPID_CISCO_LMI;
		break;

	case cpu_to_be16(ETH_P_IP):
		head_len = 4;
		skb_push(skb, head_len);
		skb->data[3] = NLPID_IP;
		break;

	case cpu_to_be16(ETH_P_IPV6):
		head_len = 4;
		skb_push(skb, head_len);
		skb->data[3] = NLPID_IPV6;
		break;

	case cpu_to_be16(ETH_P_802_3):
		head_len = 10;
		if (skb_headroom(skb) < head_len) {
			struct sk_buff *skb2 = skb_realloc_headroom(skb,
								    head_len);
			if (!skb2)
				return -ENOBUFS;
			dev_kfree_skb(skb);
			skb = *skb_p = skb2;
		}
		skb_push(skb, head_len);
		skb->data[3] = FR_PAD;
		skb->data[4] = NLPID_SNAP;
		skb->data[5] = FR_PAD;
		skb->data[6] = 0x80;
		skb->data[7] = 0xC2;
		skb->data[8] = 0x00;
		skb->data[9] = 0x07; /* bridged Ethernet frame w/out FCS */
		break;

	default:
		head_len = 10;
		skb_push(skb, head_len);
		skb->data[3] = FR_PAD;
		skb->data[4] = NLPID_SNAP;
		skb->data[5] = FR_PAD;
		skb->data[6] = FR_PAD;
		skb->data[7] = FR_PAD;
		*(__be16*)(skb->data + 8) = skb->protocol;
	}

	dlci_to_q922(skb->data, dlci);
	skb->data[2] = FR_UI;
	return 0;
}

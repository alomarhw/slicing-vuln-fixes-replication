netdev_tx_t ieee80211_subif_start_xmit(struct sk_buff *skb,
				    struct net_device *dev)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_tx_info *info;
	int head_need;
	u16 ethertype, hdrlen,  meshhdrlen = 0;
	__le16 fc;
	struct ieee80211_hdr hdr;
	struct ieee80211s_hdr mesh_hdr __maybe_unused;
	struct mesh_path __maybe_unused *mppath = NULL, *mpath = NULL;
	const u8 *encaps_data;
	int encaps_len, skip_header_bytes;
	int nh_pos, h_pos;
	struct sta_info *sta = NULL;
	bool wme_sta = false, authorized = false, tdls_auth = false;
	bool tdls_direct = false;
	bool multicast;
	u32 info_flags = 0;
	u16 info_id = 0;
	struct ieee80211_chanctx_conf *chanctx_conf;
	struct ieee80211_sub_if_data *ap_sdata;
	enum ieee80211_band band;

	if (unlikely(skb->len < ETH_HLEN))
		goto fail;

	/* convert Ethernet header to proper 802.11 header (based on
	 * operation mode) */
	ethertype = (skb->data[12] << 8) | skb->data[13];
	fc = cpu_to_le16(IEEE80211_FTYPE_DATA | IEEE80211_STYPE_DATA);

	rcu_read_lock();

	/* Measure frame arrival for Tx latency statistics calculation */
	ieee80211_tx_latency_start_msrmnt(local, skb);

	switch (sdata->vif.type) {
	case NL80211_IFTYPE_AP_VLAN:
		sta = rcu_dereference(sdata->u.vlan.sta);
		if (sta) {
			fc |= cpu_to_le16(IEEE80211_FCTL_FROMDS | IEEE80211_FCTL_TODS);
			/* RA TA DA SA */
			memcpy(hdr.addr1, sta->sta.addr, ETH_ALEN);
			memcpy(hdr.addr2, sdata->vif.addr, ETH_ALEN);
			memcpy(hdr.addr3, skb->data, ETH_ALEN);
			memcpy(hdr.addr4, skb->data + ETH_ALEN, ETH_ALEN);
			hdrlen = 30;
			authorized = test_sta_flag(sta, WLAN_STA_AUTHORIZED);
			wme_sta = test_sta_flag(sta, WLAN_STA_WME);
		}
		ap_sdata = container_of(sdata->bss, struct ieee80211_sub_if_data,
					u.ap);
		chanctx_conf = rcu_dereference(ap_sdata->vif.chanctx_conf);
		if (!chanctx_conf)
			goto fail_rcu;
		band = chanctx_conf->def.chan->band;
		if (sta)
			break;
		/* fall through */
	case NL80211_IFTYPE_AP:
		if (sdata->vif.type == NL80211_IFTYPE_AP)
			chanctx_conf = rcu_dereference(sdata->vif.chanctx_conf);
		if (!chanctx_conf)
			goto fail_rcu;
		fc |= cpu_to_le16(IEEE80211_FCTL_FROMDS);
		/* DA BSSID SA */
		memcpy(hdr.addr1, skb->data, ETH_ALEN);
		memcpy(hdr.addr2, sdata->vif.addr, ETH_ALEN);
		memcpy(hdr.addr3, skb->data + ETH_ALEN, ETH_ALEN);
		hdrlen = 24;
		band = chanctx_conf->def.chan->band;
		break;
	case NL80211_IFTYPE_WDS:
		fc |= cpu_to_le16(IEEE80211_FCTL_FROMDS | IEEE80211_FCTL_TODS);
		/* RA TA DA SA */
		memcpy(hdr.addr1, sdata->u.wds.remote_addr, ETH_ALEN);
		memcpy(hdr.addr2, sdata->vif.addr, ETH_ALEN);
		memcpy(hdr.addr3, skb->data, ETH_ALEN);
		memcpy(hdr.addr4, skb->data + ETH_ALEN, ETH_ALEN);
		hdrlen = 30;
		/*
		 * This is the exception! WDS style interfaces are prohibited
		 * when channel contexts are in used so this must be valid
		 */
		band = local->hw.conf.chandef.chan->band;
		break;
#ifdef CONFIG_MAC80211_MESH
	case NL80211_IFTYPE_MESH_POINT:
		if (!is_multicast_ether_addr(skb->data)) {
			struct sta_info *next_hop;
			bool mpp_lookup = true;

			mpath = mesh_path_lookup(sdata, skb->data);
			if (mpath) {
				mpp_lookup = false;
				next_hop = rcu_dereference(mpath->next_hop);
				if (!next_hop ||
				    !(mpath->flags & (MESH_PATH_ACTIVE |
						      MESH_PATH_RESOLVING)))
					mpp_lookup = true;
			}

			if (mpp_lookup)
				mppath = mpp_path_lookup(sdata, skb->data);

			if (mppath && mpath)
				mesh_path_del(mpath->sdata, mpath->dst);
		}

		/*
		 * Use address extension if it is a packet from
		 * another interface or if we know the destination
		 * is being proxied by a portal (i.e. portal address
		 * differs from proxied address)
		 */
		if (ether_addr_equal(sdata->vif.addr, skb->data + ETH_ALEN) &&
		    !(mppath && !ether_addr_equal(mppath->mpp, skb->data))) {
			hdrlen = ieee80211_fill_mesh_addresses(&hdr, &fc,
					skb->data, skb->data + ETH_ALEN);
			meshhdrlen = ieee80211_new_mesh_header(sdata, &mesh_hdr,
							       NULL, NULL);
		} else {
			/* DS -> MBSS (802.11-2012 13.11.3.3).
			 * For unicast with unknown forwarding information,
			 * destination might be in the MBSS or if that fails
			 * forwarded to another mesh gate. In either case
			 * resolution will be handled in ieee80211_xmit(), so
			 * leave the original DA. This also works for mcast */
			const u8 *mesh_da = skb->data;

			if (mppath)
				mesh_da = mppath->mpp;
			else if (mpath)
				mesh_da = mpath->dst;

			hdrlen = ieee80211_fill_mesh_addresses(&hdr, &fc,
					mesh_da, sdata->vif.addr);
			if (is_multicast_ether_addr(mesh_da))
				/* DA TA mSA AE:SA */
				meshhdrlen = ieee80211_new_mesh_header(
						sdata, &mesh_hdr,
						skb->data + ETH_ALEN, NULL);
			else
				/* RA TA mDA mSA AE:DA SA */
				meshhdrlen = ieee80211_new_mesh_header(
						sdata, &mesh_hdr, skb->data,
						skb->data + ETH_ALEN);

		}
		chanctx_conf = rcu_dereference(sdata->vif.chanctx_conf);
		if (!chanctx_conf)
			goto fail_rcu;
		band = chanctx_conf->def.chan->band;
		break;
#endif
	case NL80211_IFTYPE_STATION:
		if (sdata->wdev.wiphy->flags & WIPHY_FLAG_SUPPORTS_TDLS) {
			bool tdls_peer = false;

			sta = sta_info_get(sdata, skb->data);
			if (sta) {
				authorized = test_sta_flag(sta,
							WLAN_STA_AUTHORIZED);
				wme_sta = test_sta_flag(sta, WLAN_STA_WME);
				tdls_peer = test_sta_flag(sta,
							 WLAN_STA_TDLS_PEER);
				tdls_auth = test_sta_flag(sta,
						WLAN_STA_TDLS_PEER_AUTH);
			}

			/*
			 * If the TDLS link is enabled, send everything
			 * directly. Otherwise, allow TDLS setup frames
			 * to be transmitted indirectly.
			 */
			tdls_direct = tdls_peer && (tdls_auth ||
				 !(ethertype == ETH_P_TDLS && skb->len > 14 &&
				   skb->data[14] == WLAN_TDLS_SNAP_RFTYPE));
		}

		if (tdls_direct) {
			/* link during setup - throw out frames to peer */
			if (!tdls_auth)
				goto fail_rcu;

			/* DA SA BSSID */
			memcpy(hdr.addr1, skb->data, ETH_ALEN);
			memcpy(hdr.addr2, skb->data + ETH_ALEN, ETH_ALEN);
			memcpy(hdr.addr3, sdata->u.mgd.bssid, ETH_ALEN);
			hdrlen = 24;
		}  else if (sdata->u.mgd.use_4addr &&
			    cpu_to_be16(ethertype) != sdata->control_port_protocol) {
			fc |= cpu_to_le16(IEEE80211_FCTL_FROMDS |
					  IEEE80211_FCTL_TODS);
			/* RA TA DA SA */
			memcpy(hdr.addr1, sdata->u.mgd.bssid, ETH_ALEN);
			memcpy(hdr.addr2, sdata->vif.addr, ETH_ALEN);
			memcpy(hdr.addr3, skb->data, ETH_ALEN);
			memcpy(hdr.addr4, skb->data + ETH_ALEN, ETH_ALEN);
			hdrlen = 30;
		} else {
			fc |= cpu_to_le16(IEEE80211_FCTL_TODS);
			/* BSSID SA DA */
			memcpy(hdr.addr1, sdata->u.mgd.bssid, ETH_ALEN);
			memcpy(hdr.addr2, skb->data + ETH_ALEN, ETH_ALEN);
			memcpy(hdr.addr3, skb->data, ETH_ALEN);
			hdrlen = 24;
		}
		chanctx_conf = rcu_dereference(sdata->vif.chanctx_conf);
		if (!chanctx_conf)
			goto fail_rcu;
		band = chanctx_conf->def.chan->band;
		break;
	case NL80211_IFTYPE_ADHOC:
		/* DA SA BSSID */
		memcpy(hdr.addr1, skb->data, ETH_ALEN);
		memcpy(hdr.addr2, skb->data + ETH_ALEN, ETH_ALEN);
		memcpy(hdr.addr3, sdata->u.ibss.bssid, ETH_ALEN);
		hdrlen = 24;
		chanctx_conf = rcu_dereference(sdata->vif.chanctx_conf);
		if (!chanctx_conf)
			goto fail_rcu;
		band = chanctx_conf->def.chan->band;
		break;
	default:
		goto fail_rcu;
	}

	/*
	 * There's no need to try to look up the destination
	 * if it is a multicast address (which can only happen
	 * in AP mode)
	 */
	multicast = is_multicast_ether_addr(hdr.addr1);
	if (!multicast) {
		sta = sta_info_get(sdata, hdr.addr1);
		if (sta) {
			authorized = test_sta_flag(sta, WLAN_STA_AUTHORIZED);
			wme_sta = test_sta_flag(sta, WLAN_STA_WME);
		}
	}

	/* For mesh, the use of the QoS header is mandatory */
	if (ieee80211_vif_is_mesh(&sdata->vif))
		wme_sta = true;

	/* receiver and we are QoS enabled, use a QoS type frame */
	if (wme_sta && local->hw.queues >= IEEE80211_NUM_ACS) {
		fc |= cpu_to_le16(IEEE80211_STYPE_QOS_DATA);
		hdrlen += 2;
	}

	/*
	 * Drop unicast frames to unauthorised stations unless they are
	 * EAPOL frames from the local station.
	 */
	if (unlikely(!ieee80211_vif_is_mesh(&sdata->vif) &&
		     !multicast && !authorized &&
		     (cpu_to_be16(ethertype) != sdata->control_port_protocol ||
		      !ether_addr_equal(sdata->vif.addr, skb->data + ETH_ALEN)))) {
#ifdef CONFIG_MAC80211_VERBOSE_DEBUG
		net_info_ratelimited("%s: dropped frame to %pM (unauthorized port)\n",
				    dev->name, hdr.addr1);
#endif

		I802_DEBUG_INC(local->tx_handlers_drop_unauth_port);

		goto fail_rcu;
	}

	if (unlikely(!multicast && skb->sk &&
		     skb_shinfo(skb)->tx_flags & SKBTX_WIFI_STATUS)) {
		struct sk_buff *orig_skb = skb;

		skb = skb_clone(skb, GFP_ATOMIC);
		if (skb) {
			unsigned long flags;
			int id;

			spin_lock_irqsave(&local->ack_status_lock, flags);
			id = idr_alloc(&local->ack_status_frames, orig_skb,
				       1, 0x10000, GFP_ATOMIC);
			spin_unlock_irqrestore(&local->ack_status_lock, flags);

			if (id >= 0) {
				info_id = id;
				info_flags |= IEEE80211_TX_CTL_REQ_TX_STATUS;
			} else if (skb_shared(skb)) {
				kfree_skb(orig_skb);
			} else {
				kfree_skb(skb);
				skb = orig_skb;
			}
		} else {
			/* couldn't clone -- lose tx status ... */
			skb = orig_skb;
		}
	}

	/*
	 * If the skb is shared we need to obtain our own copy.
	 */
	if (skb_shared(skb)) {
		struct sk_buff *tmp_skb = skb;

		/* can't happen -- skb is a clone if info_id != 0 */
		WARN_ON(info_id);

		skb = skb_clone(skb, GFP_ATOMIC);
		kfree_skb(tmp_skb);

		if (!skb)
			goto fail_rcu;
	}

	hdr.frame_control = fc;
	hdr.duration_id = 0;
	hdr.seq_ctrl = 0;

	skip_header_bytes = ETH_HLEN;
	if (ethertype == ETH_P_AARP || ethertype == ETH_P_IPX) {
		encaps_data = bridge_tunnel_header;
		encaps_len = sizeof(bridge_tunnel_header);
		skip_header_bytes -= 2;
	} else if (ethertype >= ETH_P_802_3_MIN) {
		encaps_data = rfc1042_header;
		encaps_len = sizeof(rfc1042_header);
		skip_header_bytes -= 2;
	} else {
		encaps_data = NULL;
		encaps_len = 0;
	}

	nh_pos = skb_network_header(skb) - skb->data;
	h_pos = skb_transport_header(skb) - skb->data;

	skb_pull(skb, skip_header_bytes);
	nh_pos -= skip_header_bytes;
	h_pos -= skip_header_bytes;

	head_need = hdrlen + encaps_len + meshhdrlen - skb_headroom(skb);

	/*
	 * So we need to modify the skb header and hence need a copy of
	 * that. The head_need variable above doesn't, so far, include
	 * the needed header space that we don't need right away. If we
	 * can, then we don't reallocate right now but only after the
	 * frame arrives at the master device (if it does...)
	 *
	 * If we cannot, however, then we will reallocate to include all
	 * the ever needed space. Also, if we need to reallocate it anyway,
	 * make it big enough for everything we may ever need.
	 */

	if (head_need > 0 || skb_cloned(skb)) {
		head_need += sdata->encrypt_headroom;
		head_need += local->tx_headroom;
		head_need = max_t(int, 0, head_need);
		if (ieee80211_skb_resize(sdata, skb, head_need, true)) {
			ieee80211_free_txskb(&local->hw, skb);
			skb = NULL;
			goto fail_rcu;
		}
	}

	if (encaps_data) {
		memcpy(skb_push(skb, encaps_len), encaps_data, encaps_len);
		nh_pos += encaps_len;
		h_pos += encaps_len;
	}

#ifdef CONFIG_MAC80211_MESH
	if (meshhdrlen > 0) {
		memcpy(skb_push(skb, meshhdrlen), &mesh_hdr, meshhdrlen);
		nh_pos += meshhdrlen;
		h_pos += meshhdrlen;
	}
#endif

	if (ieee80211_is_data_qos(fc)) {
		__le16 *qos_control;

		qos_control = (__le16 *) skb_push(skb, 2);
		memcpy(skb_push(skb, hdrlen - 2), &hdr, hdrlen - 2);
		/*
		 * Maybe we could actually set some fields here, for now just
		 * initialise to zero to indicate no special operation.
		 */
		*qos_control = 0;
	} else
		memcpy(skb_push(skb, hdrlen), &hdr, hdrlen);

	nh_pos += hdrlen;
	h_pos += hdrlen;

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;

	/* Update skb pointers to various headers since this modified frame
	 * is going to go through Linux networking code that may potentially
	 * need things like pointer to IP header. */
	skb_set_mac_header(skb, 0);
	skb_set_network_header(skb, nh_pos);
	skb_set_transport_header(skb, h_pos);

	info = IEEE80211_SKB_CB(skb);
	memset(info, 0, sizeof(*info));

	dev->trans_start = jiffies;

	info->flags = info_flags;
	info->ack_frame_id = info_id;

	ieee80211_xmit(sdata, skb, band);
	rcu_read_unlock();

	return NETDEV_TX_OK;

 fail_rcu:
	rcu_read_unlock();
 fail:
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

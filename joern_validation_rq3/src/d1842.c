tBTA_AV_FEAT bta_avk_check_peer_features(uint16_t service_uuid) {
  tBTA_AV_FEAT peer_features = 0;
  tBTA_AV_CB* p_cb = &bta_av_cb;

  APPL_TRACE_DEBUG("%s: service_uuid:x%x", __func__, service_uuid);

 /* loop through all records we found */
  tSDP_DISC_REC* p_rec =
      SDP_FindServiceInDb(p_cb->p_disc_db, service_uuid, NULL);
 while (p_rec) {
    APPL_TRACE_DEBUG("%s: found Service record for x%x", __func__,
                     service_uuid);

 if ((SDP_FindAttributeInRec(p_rec, ATTR_ID_SERVICE_CLASS_ID_LIST)) !=
        NULL) {
 /* find peer features */
 if (SDP_FindServiceInDb(p_cb->p_disc_db, UUID_SERVCLASS_AV_REMOTE_CONTROL,
                              NULL)) {
        peer_features |= BTA_AV_FEAT_RCCT;
 }
 if (SDP_FindServiceInDb(p_cb->p_disc_db,
                              UUID_SERVCLASS_AV_REM_CTRL_TARGET, NULL)) {
        peer_features |= BTA_AV_FEAT_RCTG;
 }
 }

 if ((SDP_FindAttributeInRec(p_rec, ATTR_ID_BT_PROFILE_DESC_LIST)) != NULL) {
 /* get profile version (if failure, version parameter is not updated) */
 uint16_t peer_rc_version = 0;
 bool val = SDP_FindProfileVersionInRec(
          p_rec, UUID_SERVCLASS_AV_REMOTE_CONTROL, &peer_rc_version);
      APPL_TRACE_DEBUG("%s: peer_rc_version for TG 0x%x, profile_found %d",
                       __func__, peer_rc_version, val);

 if (peer_rc_version >= AVRC_REV_1_3)
        peer_features |= (BTA_AV_FEAT_VENDOR | BTA_AV_FEAT_METADATA);

 /*
       * Though Absolute Volume came after in 1.4 and above, but there are few
       * devices
       * in market which supports absolute Volume and they are still 1.3
       * TO avoid IOT issuses with those devices, we check for 1.3 as minimum
       * version
       */
 if (peer_rc_version >= AVRC_REV_1_3) {
 /* get supported features */
        tSDP_DISC_ATTR* p_attr =
            SDP_FindAttributeInRec(p_rec, ATTR_ID_SUPPORTED_FEATURES);
 if (p_attr != NULL) {
 uint16_t categories = p_attr->attr_value.v.u16;
 if (categories & AVRC_SUPF_CT_CAT2)
            peer_features |= (BTA_AV_FEAT_ADV_CTRL);
 if (categories & AVRC_SUPF_CT_APP_SETTINGS)
            peer_features |= (BTA_AV_FEAT_APP_SETTING);
 if (categories & AVRC_SUPF_CT_BROWSE)
            peer_features |= (BTA_AV_FEAT_BROWSE);
 }
 }
 }
 /* get next record; if none found, we're done */
    p_rec = SDP_FindServiceInDb(p_cb->p_disc_db, service_uuid, p_rec);
 }
  APPL_TRACE_DEBUG("%s: peer_features:x%x", __func__, peer_features);
 return peer_features;
}

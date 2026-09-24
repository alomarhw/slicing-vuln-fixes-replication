dissect_usb_ms_control(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data)
{
    gboolean is_request;
    usb_conv_info_t *usb_conv_info;
    usb_trans_info_t *usb_trans_info;
    int offset=0;
    usb_setup_dissector dissector = NULL;
    const usb_setup_dissector_table_t *tmp;

    /* Reject the packet if data or usb_trans_info are NULL */
    if (data == NULL || ((usb_conv_info_t *)data)->usb_trans_info == NULL)
        return 0;
    usb_conv_info = (usb_conv_info_t *)data;
    usb_trans_info = usb_conv_info->usb_trans_info;

    is_request=(pinfo->srcport==NO_ENDPOINT);

    /* See if we can find a class specific dissector for this request */
    for(tmp=setup_dissectors;tmp->dissector;tmp++){
        if (tmp->request == usb_trans_info->setup.request){
            dissector=tmp->dissector;
            break;
        }
    }
    /* No we could not find any class specific dissector for this request
     * return 0 and let USB try any of the standard requests.
     */
    if(!dissector){
        return 0;
    }

    col_set_str(pinfo->cinfo, COL_PROTOCOL, "USBMS");

    col_add_fstr(pinfo->cinfo, COL_INFO, "%s %s",
        val_to_str(usb_trans_info->setup.request, setup_request_names_vals, "Unknown type %x"),
        is_request?"Request":"Response");

    if(is_request){
        proto_tree_add_item(tree, hf_usb_ms_request, tvb, offset, 1, ENC_LITTLE_ENDIAN);
        offset += 1;
    }

    dissector(pinfo, tree, tvb, offset, is_request, usb_trans_info, usb_conv_info);
    return tvb_captured_length(tvb);
}

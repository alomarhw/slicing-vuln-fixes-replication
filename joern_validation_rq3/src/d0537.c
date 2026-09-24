static u32 merge_hevc_config(GF_ISOFile *dest, u32 tk_id, GF_ISOFile *orig, u32 src_track, Bool force_cat)
{
	u32 i;
	GF_HEVCConfig *hevc_src, *hevc_dst;
	u32 dst_tk = gf_isom_get_track_by_id(dest, tk_id);

	hevc_src = gf_isom_hevc_config_get(orig, src_track, 1);
	hevc_dst = gf_isom_hevc_config_get(dest, dst_tk, 1);

	if (hevc_src->profile_idc != hevc_dst->profile_idc) dst_tk = 0;
	else if (hevc_src->level_idc != hevc_dst->level_idc) dst_tk = 0;
	else if (hevc_src->general_profile_compatibility_flags != hevc_dst->general_profile_compatibility_flags ) dst_tk = 0;
	else {
		/*rewrite all samples if using different NALU size*/
		if (hevc_src->nal_unit_size > hevc_dst->nal_unit_size) {
			gf_media_avc_rewrite_samples(dest, dst_tk, 8*hevc_dst->nal_unit_size, 8*hevc_src->nal_unit_size);
			hevc_dst->nal_unit_size = hevc_src->nal_unit_size;
		} else if (hevc_src->nal_unit_size < hevc_dst->nal_unit_size) {
			gf_media_avc_rewrite_samples(orig, src_track, 8*hevc_src->nal_unit_size, 8*hevc_dst->nal_unit_size);
		}

		/*merge PS*/
		for (i=0; i<gf_list_count(hevc_src->param_array); i++) {
			u32 k;
			GF_HEVCParamArray *src_ar = gf_list_get(hevc_src->param_array, i);
			for (k=0; k<gf_list_count(hevc_dst->param_array); k++) {
				GF_HEVCParamArray *dst_ar = gf_list_get(hevc_dst->param_array, k);
				if (dst_ar->type==src_ar->type) {
					if (!merge_parameter_set(src_ar->nalus, dst_ar->nalus, "SPS"))
						dst_tk = 0;
					break;
				}
			}
		}

		gf_isom_hevc_config_update(dest, dst_tk, 1, hevc_dst);
	}

	gf_odf_hevc_cfg_del(hevc_src);
	gf_odf_hevc_cfg_del(hevc_dst);

	if (!dst_tk) {
		dst_tk = gf_isom_get_track_by_id(dest, tk_id);
		gf_isom_set_nalu_extract_mode(orig, src_track, GF_ISOM_NALU_EXTRACT_INBAND_PS_FLAG);
		if (!force_cat) {
			gf_isom_hevc_set_inband_config(dest, dst_tk, 1);
		} else {
			fprintf(stderr, "WARNING: Concatenating track ID %d even though sample descriptions do not match\n", tk_id);
		}
	}
	return dst_tk;
}

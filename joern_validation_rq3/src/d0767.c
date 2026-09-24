spnego_gss_export_name_composite(OM_uint32 *minor_status,
				 gss_name_t name,
				 gss_buffer_t exp_composite_name)
{
	OM_uint32 ret;
	ret = gss_export_name_composite(minor_status,
					name,
					exp_composite_name);
	return (ret);
}

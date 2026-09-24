spnego_gss_display_name(
			OM_uint32 *minor_status,
			gss_name_t input_name,
			gss_buffer_t output_name_buffer,
			gss_OID *output_name_type)
{
	OM_uint32 status = GSS_S_COMPLETE;
	dsyslog("Entering display_name\n");

	status = gss_display_name(minor_status, input_name,
			output_name_buffer, output_name_type);

	dsyslog("Leaving display_name\n");
	return (status);
}

BOOL nsc_context_set_pixel_format(NSC_CONTEXT* context, UINT32 pixel_format)
{
	if (!context)
		return FALSE;

	context->format = pixel_format;
	return TRUE;
}

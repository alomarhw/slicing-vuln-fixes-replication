rdp_out_unistr_mandatory_null(STREAM s, char *string, int len)
{
	/* LEGACY:
	 *
	 *  Do not write new code that uses this function, use the ones defined
	 *  in stream.h for writing utf16 strings to a stream.
	 *
	 */
	if (string && len > 0)
		rdp_out_unistr(s, string, len);
	else
		out_uint16_le(s, 0);
}

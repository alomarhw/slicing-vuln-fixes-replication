pdf_xobject_bbox(fz_context *ctx, pdf_xobject *xobj, fz_rect *bbox)
{
	return pdf_to_rect(ctx, pdf_dict_get(ctx, xobj->obj, PDF_NAME_BBox), bbox);
}

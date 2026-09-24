static void jp2_cmap_destroy(jp2_box_t *box)
{
	jp2_cmap_t *cmap = &box->data.cmap;
	if (cmap->ents) {
		jas_free(cmap->ents);
	}
}

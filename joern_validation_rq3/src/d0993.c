pdf_drop_document_imp(fz_context *ctx, pdf_document *doc)
{
	int i;

	fz_defer_reap_start(ctx);

	/* Type3 glyphs in the glyph cache can contain pdf_obj pointers
	 * that we are about to destroy. Simplest solution is to bin the
	 * glyph cache at this point. */
	fz_try(ctx)
		fz_purge_glyph_cache(ctx);
	fz_catch(ctx)
	{
		/* Swallow error, but continue dropping */
	}

	pdf_drop_js(ctx, doc->js);

	pdf_drop_xref_sections(ctx, doc);
	fz_free(ctx, doc->xref_index);

	pdf_drop_obj(ctx, doc->focus_obj);
	fz_drop_stream(ctx, doc->file);
	pdf_drop_crypt(ctx, doc->crypt);

	pdf_drop_obj(ctx, doc->linear_obj);
	if (doc->linear_page_refs)
	{
		for (i=0; i < doc->linear_page_count; i++)
			pdf_drop_obj(ctx, doc->linear_page_refs[i]);

		fz_free(ctx, doc->linear_page_refs);
	}

	fz_free(ctx, doc->hint_page);
	fz_free(ctx, doc->hint_shared_ref);
	fz_free(ctx, doc->hint_shared);
	fz_free(ctx, doc->hint_obj_offsets);

	for (i=0; i < doc->num_type3_fonts; i++)
	{
		fz_try(ctx)
			fz_decouple_type3_font(ctx, doc->type3_fonts[i], (void *)doc);
		fz_always(ctx)
			fz_drop_font(ctx, doc->type3_fonts[i]);
		fz_catch(ctx)
		{
			/* Swallow error, but continue dropping */
		}
	}

	fz_free(ctx, doc->type3_fonts);

	pdf_drop_ocg(ctx, doc);
	pdf_drop_portfolio(ctx, doc);

	pdf_empty_store(ctx, doc);

	pdf_lexbuf_fin(ctx, &doc->lexbuf.base);

	pdf_drop_resource_tables(ctx, doc);

	fz_drop_colorspace(ctx, doc->oi);

	for (i = 0; i < doc->orphans_count; i++)
		pdf_drop_obj(ctx, doc->orphans[i]);

	fz_free(ctx, doc->orphans);

	fz_free(ctx, doc->rev_page_map);

	fz_defer_reap_end(ctx);
}

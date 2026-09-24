static void compute_string_bbox(TextInfo *text, DBBox *bbox)
{
    int i;

    if (text->length > 0) {
        bbox->xMin = 32000;
        bbox->xMax = -32000;
        bbox->yMin = -1 * text->lines[0].asc + d6_to_double(text->glyphs[0].pos.y);
        bbox->yMax = text->height - text->lines[0].asc +
                     d6_to_double(text->glyphs[0].pos.y);

        for (i = 0; i < text->length; ++i) {
            GlyphInfo *info = text->glyphs + i;
            if (info->skip) continue;
            double s = d6_to_double(info->pos.x);
            double e = s + d6_to_double(info->cluster_advance.x);
            bbox->xMin = FFMIN(bbox->xMin, s);
            bbox->xMax = FFMAX(bbox->xMax, e);
        }
    } else
        bbox->xMin = bbox->xMax = bbox->yMin = bbox->yMax = 0.;
}

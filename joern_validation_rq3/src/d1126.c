static void WarnPatented(gs_font_type42 *pfont, ttfFont *ttf, const char *txt)
{
    if (!ttf->design_grid) {
        char buf[gs_font_name_max + 1];
        int l;
        gs_font_type42 *base_font = pfont;

        while ((gs_font_type42 *)base_font->base != base_font)
            base_font = (gs_font_type42 *)base_font->base;
        if (!base_font->data.warning_patented) {
            l = min(sizeof(buf) - 1, base_font->font_name.size);
            memcpy(buf, base_font->font_name.chars, l);
            buf[l] = 0;
            emprintf2(pfont->memory,
                      "%s %s requires a patented True Type interpreter.\n",
                      txt,
                      buf);
            base_font->data.warning_patented = true;
        }
    }
}

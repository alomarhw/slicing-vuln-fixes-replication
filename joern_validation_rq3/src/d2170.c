views::Label* CreateInfoLabel() {
  views::Label* label = new views::Label();
  label->SetAutoColorReadabilityEnabled(false);
  label->SetEnabledColor(SK_ColorWHITE);
  label->SetFontList(views::Label::GetDefaultFontList().Derive(
      -1, gfx::Font::FontStyle::NORMAL, gfx::Font::Weight::NORMAL));
  label->SetSubpixelRenderingEnabled(false);

  return label;
}

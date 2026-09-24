bool CSSPaintValue::KnownToBeOpaque(const Document&,
                                    const ComputedStyle&) const {
  return generator_ && !generator_->HasAlpha();
}

bool LayoutBlockFlow::containsFloat(LayoutBox* layoutBox) const
{
    return m_floatingObjects && m_floatingObjects->set().contains<FloatingObjectHashTranslator>(layoutBox);
}

void Document::setDesignMode(const String& value)
{
    bool newValue = m_designMode;
    if (equalIgnoringCase(value, "on")) {
        newValue = true;
        UseCounter::count(*this, UseCounter::DocumentDesignModeEnabeld);
    } else if (equalIgnoringCase(value, "off")) {
        newValue = false;
    }
    if (newValue == m_designMode)
        return;
    m_designMode = newValue;
    setNeedsStyleRecalc(SubtreeStyleChange, StyleChangeReasonForTracing::create(StyleChangeReason::DesignMode));
}

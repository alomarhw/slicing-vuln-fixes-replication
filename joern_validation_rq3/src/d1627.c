bool HTMLFormControlElement::isValidElement()
{
    if (m_validityIsDirty) {
        m_isValid = !willValidate() || valid();
        m_validityIsDirty = false;
    } else {
        ASSERT(m_isValid == (!willValidate() || valid()));
    }
    return m_isValid;
}

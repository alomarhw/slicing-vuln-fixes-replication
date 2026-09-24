bool AccessibilityUIElement::isEqual(AccessibilityUIElement* otherElement)
{
    return m_element == otherElement->platformUIElement();
}

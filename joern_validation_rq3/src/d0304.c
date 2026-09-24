Element* Document::createElementNS(const AtomicString& namespaceURI, const AtomicString& qualifiedName, const AtomicString& typeExtension, ExceptionState& exceptionState)
{
    QualifiedName qName(createQualifiedName(namespaceURI, qualifiedName, exceptionState));
    if (qName == QualifiedName::null())
        return nullptr;

    Element* element;
    if (CustomElement::shouldCreateCustomElement(*this, qName))
        element = CustomElement::createCustomElementSync(*this, qName, exceptionState);
    else if (V0CustomElement::isValidName(qName.localName()) && registrationContext())
        element = registrationContext()->createCustomTagElement(*this, qName);
    else
        element = createElement(qName, CreatedByCreateElement);

    if (!typeExtension.isEmpty())
        V0CustomElementRegistrationContext::setIsAttributeAndTypeExtension(element, typeExtension);

    return element;
}

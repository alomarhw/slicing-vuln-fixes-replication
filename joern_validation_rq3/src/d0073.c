bool SVGFEColorMatrixElement::isSupportedAttribute(const QualifiedName& attrName)
{
    DEFINE_STATIC_LOCAL(HashSet<QualifiedName>, supportedAttributes, ());
    if (supportedAttributes.isEmpty()) {
        supportedAttributes.add(SVGNames::typeAttr);
        supportedAttributes.add(SVGNames::valuesAttr);
        supportedAttributes.add(SVGNames::inAttr);
    }
    return supportedAttributes.contains<SVGAttributeHashTranslator>(attrName);
}

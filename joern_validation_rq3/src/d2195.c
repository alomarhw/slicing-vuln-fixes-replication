bool HTMLTextAreaElement::isPresentationAttribute(const QualifiedName& name) const
{
    if (name == alignAttr) {
        return false;
    }

    if (name == wrapAttr)
        return true;
    return HTMLTextFormControlElement::isPresentationAttribute(name);
}

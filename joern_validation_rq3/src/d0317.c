String WebPageSerializerImpl::preActionBeforeSerializeOpenTag(
    const Element* element, SerializeDomParam* param, bool* needSkip)
{
    StringBuilder result;

    *needSkip = false;
    if (param->isHTMLDocument) {
        ASSERT(element);
        if (isHTMLMetaElement(*element)) {
            const HTMLMetaElement& meta = toHTMLMetaElement(*element);
            String equiv = meta.httpEquiv();
            if (equalIgnoringCase(equiv, "content-type")) {
                String content = meta.content();
                if (content.length() && content.contains("charset", TextCaseInsensitive)) {
                    param->skipMetaElement = element;
                    *needSkip = true;
                }
            }
        } else if (isHTMLHtmlElement(*element)) {
            if (!param->haveSeenDocType) {
                param->haveSeenDocType = true;
                result.append(createMarkup(param->document->doctype()));
            }

            result.append(WebPageSerializer::generateMarkOfTheWebDeclaration(param->url));
        } else if (isHTMLBaseElement(*element)) {
            result.appendLiteral("<!--");
        }
    } else {
        if (!param->haveAddedXMLProcessingDirective) {
            param->haveAddedXMLProcessingDirective = true;
            String xmlEncoding = param->document->xmlEncoding();
            if (xmlEncoding.isEmpty())
                xmlEncoding = param->document->encodingName();
            if (xmlEncoding.isEmpty())
                xmlEncoding = UTF8Encoding().name();
            result.appendLiteral("<?xml version=\"");
            result.append(param->document->xmlVersion());
            result.appendLiteral("\" encoding=\"");
            result.append(xmlEncoding);
            if (param->document->xmlStandalone())
                result.appendLiteral("\" standalone=\"yes");
            result.appendLiteral("\"?>\n");
        }
        if (!param->haveSeenDocType) {
            param->haveSeenDocType = true;
            result.append(createMarkup(param->document->doctype()));
        }
    }
    return result.toString();
}

void XMLHttpRequest::setResponseType(const String& responseType, ExceptionState& es)
{
    if (m_state >= LOADING) {
        es.throwDOMException(InvalidStateError, ExceptionMessages::failedToSet("responseType", "XMLHttpRequest", "the response type cannot be set if the object's state is LOADING or DONE."));
        return;
    }

    if (!m_async && scriptExecutionContext()->isDocument() && m_url.protocolIsInHTTPFamily()) {
        es.throwDOMException(InvalidAccessError, ExceptionMessages::failedToSet("responseType", "XMLHttpRequest", "the response type can only be changed for asynchronous HTTP requests made from a document."));
        return;
    }

    if (responseType == "") {
        m_responseTypeCode = ResponseTypeDefault;
    } else if (responseType == "text") {
        m_responseTypeCode = ResponseTypeText;
    } else if (responseType == "json") {
        m_responseTypeCode = ResponseTypeJSON;
    } else if (responseType == "document") {
        m_responseTypeCode = ResponseTypeDocument;
    } else if (responseType == "blob") {
        m_responseTypeCode = ResponseTypeBlob;
    } else if (responseType == "arraybuffer") {
        m_responseTypeCode = ResponseTypeArrayBuffer;
    } else if (responseType == "stream") {
        if (RuntimeEnabledFeatures::streamEnabled())
            m_responseTypeCode = ResponseTypeStream;
        else
            return;
    } else {
        ASSERT_NOT_REACHED();
    }
}

bool FrameLoader::isMixedContent(SecurityOrigin* context, const KURL& url)
{
    if (context->protocol() != "https")
        return false;  // We only care about HTTPS security origins.

    if (url.protocolIs("https") || url.protocolIs("about") || url.protocolIs("data"))
        return false;  // Loading these protocols is secure.

    return true;
}

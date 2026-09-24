void FrameLoaderClient::dispatchDidFinishLoading(WebCore::DocumentLoader* loader, unsigned long identifier)
{
    static_cast<WebKit::DocumentLoader*>(loader)->decreaseLoadCount(identifier);

    WebKitWebView* webView = getViewFromFrame(m_frame);
    GOwnPtr<gchar> identifierString(toString(identifier));
    WebKitWebResource* webResource = webkit_web_view_get_resource(webView, identifierString.get());

    if (!webResource)
        return;

    const char* uri = webkit_web_resource_get_uri(webResource);
    RefPtr<ArchiveResource> coreResource(loader->subresource(KURL(KURL(), uri)));

    if (!coreResource && webResource != webkit_web_view_get_main_resource(webView))
        return;

    if (!coreResource)
        coreResource = loader->mainResource().releaseRef();

    webkit_web_resource_init_with_core_resource(webResource, coreResource.get());

    notImplemented();
}

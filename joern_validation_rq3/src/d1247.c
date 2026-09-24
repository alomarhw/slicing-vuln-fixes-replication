void BlobRegistryProxy::registerBlobURL(const KURL& url, const KURL& srcURL)
{
    if (m_webBlobRegistry)
        m_webBlobRegistry->registerBlobURL(url, srcURL);
}

bool InspectorResourceAgent::fetchResourceContent(LocalFrame* frame, const KURL& url, String* content, bool* base64Encoded)
{
    Resource* cachedResource = frame->document()->fetcher()->cachedResource(url);
    if (!cachedResource)
        cachedResource = memoryCache()->resourceForURL(url);
    if (cachedResource && InspectorPageAgent::cachedResourceContent(cachedResource, content, base64Encoded))
        return true;

    Vector<NetworkResourcesData::ResourceData*> resources = m_resourcesData->resources();
    for (Vector<NetworkResourcesData::ResourceData*>::iterator it = resources.begin(); it != resources.end(); ++it) {
        if ((*it)->url() == url) {
            *content = (*it)->content();
            *base64Encoded = (*it)->base64Encoded();
            return true;
        }
    }
     return false;
 }

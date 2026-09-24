void ResourceFetcher::didReceiveResponse(const Resource* resource, const ResourceResponse& response)
{
    context().dispatchDidReceiveResponse(m_documentLoader, resource->identifier(), response, resource->loader());
}

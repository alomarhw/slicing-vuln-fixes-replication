void DocumentThreadableLoader::handleError(const ResourceError& error)
{
    ResourceError copiedError = error;

    ThreadableLoaderClient* client = m_client;
    clear();
    client->didFail(copiedError);
}

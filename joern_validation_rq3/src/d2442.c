ResourcePtr<ScriptResource> ResourceFetcher::fetchScript(FetchRequest& request)
{
    return toScriptResource(requestResource(Resource::Script, request));
}

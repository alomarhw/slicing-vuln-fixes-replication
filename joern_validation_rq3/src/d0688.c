static PassRefPtr<JSONObject> buildObjectForHeaders(const HTTPHeaderMap& headers)
{
    RefPtr<JSONObject> headersObject = JSONObject::create();
    HTTPHeaderMap::const_iterator end = headers.end();
    for (HTTPHeaderMap::const_iterator it = headers.begin(); it != end; ++it)
        headersObject->setString(it->key.string(), it->value);
    return headersObject;
}

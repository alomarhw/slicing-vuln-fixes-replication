static bool hasPrefix(const WTF::String& searchString, const WTF::String& prefix)
{
    return searchString.length() >= prefix.length() && searchString.substring(0, prefix.length()) == prefix;
}

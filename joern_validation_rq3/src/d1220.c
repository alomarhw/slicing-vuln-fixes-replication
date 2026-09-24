bool WebPage::executeJavaScriptInIsolatedWorld(const std::wstring& script, JavaScriptDataType& returnType, BlackBerry::Platform::String& returnValue)
{
    int lengthCopied = 0;
    UErrorCode error = U_ZERO_ERROR;
    const int length = script.length() + 1 /*null termination char*/;
    UChar data[length];

    u_strFromUTF32(data, length, &lengthCopied, reinterpret_cast<const UChar32*>(script.c_str()), script.length(), &error);
    BLACKBERRY_ASSERT(error == U_ZERO_ERROR);
    if (error != U_ZERO_ERROR) {
        Platform::logAlways(Platform::LogLevelCritical, "WebPage::executeJavaScriptInIsolatedWorld failed to convert UTF16 to JavaScript!");
        return false;
    }
    String str = String(data, lengthCopied);
    ScriptSourceCode sourceCode(str, KURL());
    return d->executeJavaScriptInIsolatedWorld(sourceCode, returnType, returnValue);
}

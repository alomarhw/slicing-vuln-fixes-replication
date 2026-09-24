static int CALLBACK getLastResortFallbackFontProc(const LOGFONT* logFont, const TEXTMETRIC* metrics, DWORD fontType, LPARAM lParam)
{
    GetLastResortFallbackFontProcData* procData = reinterpret_cast<GetLastResortFallbackFontProcData*>(lParam);
    procData->m_fontData = procData->m_fontCache->fontDataFromDescriptionAndLogFont(*procData->m_fontDescription, procData->m_shouldRetain, *logFont, procData->m_fontName);
    return !procData->m_fontData;
}

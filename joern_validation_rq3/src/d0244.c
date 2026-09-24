static char* ap_lua_binstrstr (const char * haystack, size_t hsize, const char* needle, size_t nsize)
{
    size_t p;
    if (haystack == NULL) return NULL;
    if (needle == NULL) return NULL;
    if (hsize < nsize) return NULL;
    for (p = 0; p <= (hsize - nsize); ++p) {
        if (memcmp(haystack + p, needle, nsize) == 0) {
            return (char*) (haystack + p);
        }
    }
    return NULL;
} 

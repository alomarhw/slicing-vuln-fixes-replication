 bool makeAllDirectories(const String& path)
 {
    String fullPath = path;
    if (SHCreateDirectoryEx(0, fullPath.charactersWithNullTermination(), 0) != ERROR_SUCCESS) {
        DWORD error = GetLastError();
        if (error != ERROR_FILE_EXISTS && error != ERROR_ALREADY_EXISTS) {
            LOG_ERROR("Failed to create path %s", path.ascii().data());
            return false;
        }
    }
    return true;
}

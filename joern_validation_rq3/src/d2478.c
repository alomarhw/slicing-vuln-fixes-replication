static void* getProcAddress(const char* procName)
{
    static bool initialized = false;
    static glGetProcAddressType getProcAddressFunction = 0;

    if (!initialized) {
        getProcAddressFunction = reinterpret_cast<glGetProcAddressType>(dlsym(RTLD_DEFAULT, "glXGetProcAddress"));
        if (!getProcAddressFunction)
            getProcAddressFunction = reinterpret_cast<glGetProcAddressType>(dlsym(RTLD_DEFAULT, "glXGetProcAddressARB"));
    }

    if (!getProcAddressFunction)
        return dlsym(RTLD_DEFAULT, procName);
    return getProcAddressFunction(procName);
}

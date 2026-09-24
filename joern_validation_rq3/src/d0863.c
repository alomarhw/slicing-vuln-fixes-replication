__xmlGetWarningsDefaultValue(void) {
    if (IS_MAIN_THREAD)
	return (&xmlGetWarningsDefaultValue);
    else
	return (&xmlGetGlobalState()->xmlGetWarningsDefaultValue);
}

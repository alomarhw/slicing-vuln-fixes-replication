	void testQueryListHelper(const wchar_t * input, int expectedItemCount) {
		int res;

		UriBool spacePlusConversion = URI_TRUE;
		UriBool normalizeBreaks = URI_FALSE;
		UriBreakConversion breakConversion = URI_BR_DONT_TOUCH;

		int itemCount;
		UriQueryListW * queryList;
		res = uriDissectQueryMallocExW(&queryList, &itemCount,
				input, input + wcslen(input), spacePlusConversion, breakConversion);
		ASSERT_TRUE(res == URI_SUCCESS);
		ASSERT_TRUE(itemCount == expectedItemCount);
		ASSERT_TRUE((queryList == NULL) == (expectedItemCount == 0));

		if (expectedItemCount != 0) {
			int charsRequired;
			res = uriComposeQueryCharsRequiredExW(queryList, &charsRequired, spacePlusConversion,
					normalizeBreaks);
			ASSERT_TRUE(res == URI_SUCCESS);
			ASSERT_TRUE(charsRequired >= (int)wcslen(input));

			wchar_t * recomposed = new wchar_t[charsRequired + 1];
			int charsWritten;
			res = uriComposeQueryExW(recomposed, queryList, charsRequired + 1,
					&charsWritten, spacePlusConversion, normalizeBreaks);
			ASSERT_TRUE(res == URI_SUCCESS);
			ASSERT_TRUE(charsWritten <= charsRequired);
			ASSERT_TRUE(charsWritten == (int)wcslen(input) + 1);
			ASSERT_TRUE(!wcscmp(input, recomposed));
			delete [] recomposed;

			recomposed = NULL;
			res = uriComposeQueryMallocW(&recomposed, queryList);
			ASSERT_TRUE(res == URI_SUCCESS);
			ASSERT_TRUE(recomposed != NULL);
			ASSERT_TRUE(charsWritten == (int)wcslen(input) + 1);
			ASSERT_TRUE(!wcscmp(input, recomposed));
			free(recomposed);
		}

		uriFreeQueryListW(queryList);
	}

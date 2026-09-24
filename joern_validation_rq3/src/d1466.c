ImportTIFF_EncodedString ( const TIFF_Manager & tiff, const TIFF_Manager::TagInfo & tagInfo,
						   SXMPMeta * xmp, const char * xmpNS, const char * xmpProp, bool isLangAlt = false )
{
	try {	// Don't let errors with one stop the others.

		std::string strValue;

		bool ok = tiff.DecodeString ( tagInfo.dataPtr, tagInfo.dataLen, &strValue );
		if ( ! ok ) return;

		TrimTrailingSpaces ( &strValue );
		if ( strValue.empty() ) return;

		if ( ! isLangAlt ) {
			xmp->SetProperty ( xmpNS, xmpProp, strValue.c_str() );
		} else {
			xmp->SetLocalizedText ( xmpNS, xmpProp, "", "x-default", strValue.c_str() );
		}

	} catch ( ... ) {
	}

}	// ImportTIFF_EncodedString

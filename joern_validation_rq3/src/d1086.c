_zip_dirent_process_winzip_aes(zip_dirent_t *de, zip_error_t *error)
{
    zip_uint16_t ef_len;
    zip_buffer_t *buffer;
    const zip_uint8_t *ef;
    bool crc_valid;
    zip_uint16_t enc_method;


    if (de->comp_method != ZIP_CM_WINZIP_AES) {
	return true;
    }

    ef = _zip_ef_get_by_id(de->extra_fields, &ef_len, ZIP_EF_WINZIP_AES, 0, ZIP_EF_BOTH, NULL);

    if (ef == NULL || ef_len < 7) {
	zip_error_set(error, ZIP_ER_INCONS, 0);
	return false;
    }

    if ((buffer = _zip_buffer_new((zip_uint8_t *)ef, ef_len)) == NULL) {
	zip_error_set(error, ZIP_ER_INTERNAL, 0);
        return false;
    }

    /* version */

    crc_valid = true;
    switch (_zip_buffer_get_16(buffer)) {
    case 1:
	break;

    case 2:
	if (de->uncomp_size < 20 /* TODO: constant */) {
	    crc_valid = false;
	}
	break;

    default:
	zip_error_set(error, ZIP_ER_ENCRNOTSUPP, 0);
	_zip_buffer_free(buffer);
	return false;
    }

    /* vendor */
    if (memcmp(_zip_buffer_get(buffer, 2), "AE", 2) != 0) {
	zip_error_set(error, ZIP_ER_ENCRNOTSUPP, 0);
	_zip_buffer_free(buffer);
	return false;
    }

    /* mode */
    switch (_zip_buffer_get_8(buffer)) {
    case 1:
	enc_method = ZIP_EM_AES_128;
	break;
    case 2:
	enc_method = ZIP_EM_AES_192;
	break;
    case 3:
	enc_method = ZIP_EM_AES_256;
	break;
    default:
	zip_error_set(error, ZIP_ER_ENCRNOTSUPP, 0);
	_zip_buffer_free(buffer);
	return false;
    }

    if (ef_len != 7) {
	zip_error_set(error, ZIP_ER_INCONS, 0);
	_zip_buffer_free(buffer);
	return false;
    }

    de->crc_valid = crc_valid;
    de->encryption_method = enc_method;
    de->comp_method = _zip_buffer_get_16(buffer);

    _zip_buffer_free(buffer);
    return true;
}

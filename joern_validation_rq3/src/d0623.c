_gnutls_decrypt (gnutls_session_t session, uint8_t * ciphertext,
                 size_t ciphertext_size, uint8_t * data,
                 size_t max_data_size, content_type_t type,
                 record_parameters_st * params, uint64 *sequence)
{
  gnutls_datum_t gcipher;
  int ret, data_size;

  if (ciphertext_size == 0)
    return 0;

  gcipher.size = ciphertext_size;
  gcipher.data = ciphertext;

  if (is_read_comp_null (params) == 0)
    {
      ret =
        ciphertext_to_compressed (session, &gcipher, data, max_data_size,
                                   type, params, sequence);
      if (ret < 0)
        return gnutls_assert_val(ret);
      
      return ret;
    }
  else
    {
      uint8_t* tmp_data;
      
      tmp_data = gnutls_malloc(max_data_size);
      if (tmp_data == NULL)
        return gnutls_assert_val(GNUTLS_E_MEMORY_ERROR);
      
      ret =
        ciphertext_to_compressed (session, &gcipher, tmp_data, max_data_size,
                                   type, params, sequence);
      if (ret < 0)
        goto leave;
      
      data_size = ret;
        
      if (ret != 0)
        {
          ret = _gnutls_decompress( &params->read.compression_state, tmp_data, data_size, data, max_data_size);
          if (ret < 0)
            goto leave;
        }
        
leave:
      gnutls_free(tmp_data);
      return ret;
    }
}

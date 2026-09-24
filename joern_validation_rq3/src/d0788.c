bson_iter_visit_all (bson_iter_t *iter,             /* INOUT */
                     const bson_visitor_t *visitor, /* IN */
                     void *data)                    /* IN */
{
   uint32_t bson_type;
   const char *key;
   bool unsupported;

   BSON_ASSERT (iter);
   BSON_ASSERT (visitor);

   while (_bson_iter_next_internal (iter, 0, &key, &bson_type, &unsupported)) {
      if (*key && !bson_utf8_validate (key, strlen (key), false)) {
         iter->err_off = iter->off;
         break;
      }

      if (VISIT_BEFORE (iter, key, data)) {
         return true;
      }

      switch (bson_type) {
      case BSON_TYPE_DOUBLE:

         if (VISIT_DOUBLE (iter, key, bson_iter_double (iter), data)) {
            return true;
         }

         break;
      case BSON_TYPE_UTF8: {
         uint32_t utf8_len;
         const char *utf8;

         utf8 = bson_iter_utf8 (iter, &utf8_len);

         if (!bson_utf8_validate (utf8, utf8_len, true)) {
            iter->err_off = iter->off;
            return true;
         }

         if (VISIT_UTF8 (iter, key, utf8_len, utf8, data)) {
            return true;
         }
      } break;
      case BSON_TYPE_DOCUMENT: {
         const uint8_t *docbuf = NULL;
         uint32_t doclen = 0;
         bson_t b;

         bson_iter_document (iter, &doclen, &docbuf);

         if (bson_init_static (&b, docbuf, doclen) &&
             VISIT_DOCUMENT (iter, key, &b, data)) {
            return true;
         }
      } break;
      case BSON_TYPE_ARRAY: {
         const uint8_t *docbuf = NULL;
         uint32_t doclen = 0;
         bson_t b;

         bson_iter_array (iter, &doclen, &docbuf);

         if (bson_init_static (&b, docbuf, doclen) &&
             VISIT_ARRAY (iter, key, &b, data)) {
            return true;
         }
      } break;
      case BSON_TYPE_BINARY: {
         const uint8_t *binary = NULL;
         bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
         uint32_t binary_len = 0;

         bson_iter_binary (iter, &subtype, &binary_len, &binary);

         if (VISIT_BINARY (iter, key, subtype, binary_len, binary, data)) {
            return true;
         }
      } break;
      case BSON_TYPE_UNDEFINED:

         if (VISIT_UNDEFINED (iter, key, data)) {
            return true;
         }

         break;
      case BSON_TYPE_OID:

         if (VISIT_OID (iter, key, bson_iter_oid (iter), data)) {
            return true;
         }

         break;
      case BSON_TYPE_BOOL:

         if (VISIT_BOOL (iter, key, bson_iter_bool (iter), data)) {
            return true;
         }

         break;
      case BSON_TYPE_DATE_TIME:

         if (VISIT_DATE_TIME (iter, key, bson_iter_date_time (iter), data)) {
            return true;
         }

         break;
      case BSON_TYPE_NULL:

         if (VISIT_NULL (iter, key, data)) {
            return true;
         }

         break;
      case BSON_TYPE_REGEX: {
         const char *regex = NULL;
         const char *options = NULL;
         regex = bson_iter_regex (iter, &options);

         if (!bson_utf8_validate (regex, strlen (regex), true)) {
            iter->err_off = iter->off;
            return true;
         }

         if (VISIT_REGEX (iter, key, regex, options, data)) {
            return true;
         }
      } break;
      case BSON_TYPE_DBPOINTER: {
         uint32_t collection_len = 0;
         const char *collection = NULL;
         const bson_oid_t *oid = NULL;

         bson_iter_dbpointer (iter, &collection_len, &collection, &oid);

         if (!bson_utf8_validate (collection, collection_len, true)) {
            iter->err_off = iter->off;
            return true;
         }

         if (VISIT_DBPOINTER (
                iter, key, collection_len, collection, oid, data)) {
            return true;
         }
      } break;
      case BSON_TYPE_CODE: {
         uint32_t code_len;
         const char *code;

         code = bson_iter_code (iter, &code_len);

         if (!bson_utf8_validate (code, code_len, true)) {
            iter->err_off = iter->off;
            return true;
         }

         if (VISIT_CODE (iter, key, code_len, code, data)) {
            return true;
         }
      } break;
      case BSON_TYPE_SYMBOL: {
         uint32_t symbol_len;
         const char *symbol;

         symbol = bson_iter_symbol (iter, &symbol_len);

         if (!bson_utf8_validate (symbol, symbol_len, true)) {
            iter->err_off = iter->off;
            return true;
         }

         if (VISIT_SYMBOL (iter, key, symbol_len, symbol, data)) {
            return true;
         }
      } break;
      case BSON_TYPE_CODEWSCOPE: {
         uint32_t length = 0;
         const char *code;
         const uint8_t *docbuf = NULL;
         uint32_t doclen = 0;
         bson_t b;

         code = bson_iter_codewscope (iter, &length, &doclen, &docbuf);

         if (!bson_utf8_validate (code, length, true)) {
            iter->err_off = iter->off;
            return true;
         }

         if (bson_init_static (&b, docbuf, doclen) &&
             VISIT_CODEWSCOPE (iter, key, length, code, &b, data)) {
            return true;
         }
      } break;
      case BSON_TYPE_INT32:

         if (VISIT_INT32 (iter, key, bson_iter_int32 (iter), data)) {
            return true;
         }

         break;
      case BSON_TYPE_TIMESTAMP: {
         uint32_t timestamp;
         uint32_t increment;
         bson_iter_timestamp (iter, &timestamp, &increment);

         if (VISIT_TIMESTAMP (iter, key, timestamp, increment, data)) {
            return true;
         }
      } break;
      case BSON_TYPE_INT64:

         if (VISIT_INT64 (iter, key, bson_iter_int64 (iter), data)) {
            return true;
         }

         break;
      case BSON_TYPE_DECIMAL128: {
         bson_decimal128_t dec;
         bson_iter_decimal128 (iter, &dec);

         if (VISIT_DECIMAL128 (iter, key, &dec, data)) {
            return true;
         }
      } break;
      case BSON_TYPE_MAXKEY:

         if (VISIT_MAXKEY (iter, bson_iter_key_unsafe (iter), data)) {
            return true;
         }

         break;
      case BSON_TYPE_MINKEY:

         if (VISIT_MINKEY (iter, bson_iter_key_unsafe (iter), data)) {
            return true;
         }

         break;
      case BSON_TYPE_EOD:
      default:
         break;
      }

      if (VISIT_AFTER (iter, bson_iter_key_unsafe (iter), data)) {
         return true;
      }
   }

   if (iter->err_off) {
      if (unsupported && visitor->visit_unsupported_type &&
          bson_utf8_validate (key, strlen (key), false)) {
         visitor->visit_unsupported_type (iter, key, bson_type, data);
         return false;
      }

      VISIT_CORRUPT (iter, data);
   }

#undef VISIT_FIELD

   return false;
}

long Tags::Tag::Parse(IMkvReader* pReader, long long pos, long long size) {
 const long long stop = pos + size;

 while (pos < stop) {
 long long id, size;

 long status = ParseElementHeader(pReader, pos, stop, id, size);

 if (status < 0)
 return status;

 if (size == 0) // 0 length tag, read another
 continue;

 if (id == 0x27C8) { // SimpleTag ID
      status = ParseSimpleTag(pReader, pos, size);

 if (status < 0)
 return status;
 }

    pos += size;
 if (pos > stop)
 return E_FILE_FORMAT_INVALID;
 }

 if (pos != stop)
 return E_FILE_FORMAT_INVALID;
 return 0;
}

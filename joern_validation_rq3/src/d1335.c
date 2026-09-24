CMYKGrayEncoder::CMYKGrayEncoder(Stream *strA):
    FilterStream(strA) {
  bufPtr = bufEnd = buf;
  eof = gFalse;
}

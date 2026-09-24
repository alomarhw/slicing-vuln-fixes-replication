ASCIIHexEncoder::~ASCIIHexEncoder() {
  if (str->isEncoder()) {
    delete str;
  }
}

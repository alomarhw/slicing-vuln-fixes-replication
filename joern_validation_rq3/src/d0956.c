XML_SetXmlDeclHandler(XML_Parser parser, XML_XmlDeclHandler handler) {
  if (parser != NULL)
    parser->m_xmlDeclHandler = handler;
}

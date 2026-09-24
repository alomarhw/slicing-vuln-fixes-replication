raptor_rdfxml_characters_handler(void *user_data, 
                                 raptor_xml_element* xml_element,
                                 const unsigned char *s, int len)
{
  raptor_parser* rdf_parser = (raptor_parser*)user_data;

  raptor_rdfxml_cdata_grammar(rdf_parser, s, len, 0);
}

raptor_rdfxml_inscope_base_uri(raptor_parser *rdf_parser)
{
  raptor_rdfxml_parser* rdf_xml_parser;
  raptor_uri* base_uri;

  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;

  base_uri = raptor_sax2_inscope_base_uri(rdf_xml_parser->sax2);
  if(!base_uri)
    base_uri = rdf_parser->base_uri;

  return base_uri;
}

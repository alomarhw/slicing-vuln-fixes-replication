raptor_rdfxml_cdata_grammar(raptor_parser *rdf_parser,
                            const unsigned char *s, int len,
                            int is_cdata)
{
  raptor_rdfxml_parser* rdf_xml_parser;
  raptor_rdfxml_element* element;
  raptor_xml_element* xml_element;
  raptor_state state;
  int all_whitespace = 1;
  int i;

  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;

  if(rdf_parser->failed)
    return;

#ifdef RAPTOR_DEBUG_CDATA
  RAPTOR_DEBUG2("Adding characters (is_cdata=%d): '", is_cdata);
  (void)fwrite(s, 1, len, stderr);
  fprintf(stderr, "' (%d bytes)\n", len);
#endif

  for(i = 0; i < len; i++)
    if(!isspace(s[i])) {
      all_whitespace = 0;
      break;
    }

  element = rdf_xml_parser->current_element;

  /* this file is very broke - probably not XML, whatever */
  if(!element)
    return;

  xml_element = element->xml_element;
  
  raptor_rdfxml_update_document_locator(rdf_parser);

  /* cdata never changes the parser state 
   * and the containing element state always determines what to do.
   * Use the child_state first if there is one, since that applies
   */
  state = element->child_state;
#ifdef RAPTOR_DEBUG_VERBOSE
  RAPTOR_DEBUG2("Working in state %s\n", raptor_rdfxml_state_as_string(state));
#endif


#ifdef RAPTOR_DEBUG_VERBOSE
  RAPTOR_DEBUG3("Content type %s (%d)\n",
                raptor_rdfxml_element_content_type_as_string(element->content_type),
                element->content_type);
#endif
  


  if(state == RAPTOR_STATE_SKIPPING)
    return;

  if(state == RAPTOR_STATE_UNKNOWN) {
    /* Ignore all cdata if still looking for RDF */
    if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_SCANNING))
      return;

    /* Ignore all whitespace cdata before first element */
    if(all_whitespace)
      return;
    
    /* This probably will never happen since that would make the
     * XML not be well-formed
     */
    raptor_parser_warning(rdf_parser, "Character data before RDF element.");
  }


  if(element->child_content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTIES) {
    /* If found non-whitespace content, move to literal content */
    if(!all_whitespace)
      element->child_content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LITERAL; 
  }


  if(!rdf_content_type_info[element->child_content_type].whitespace_significant) {

    /* Whitespace is ignored except for literal or preserved content types */
    if(all_whitespace) {
#ifdef RAPTOR_DEBUG_CDATA
      RAPTOR_DEBUG2("Ignoring whitespace cdata inside element '%s'\n",
                    raptor_xml_element_get_name(element->parent->xml_element)->local_name);
#endif
      return;
    }

    if(xml_element->content_cdata_seen && xml_element->content_element_seen) {
      raptor_qname* parent_el_name;

      parent_el_name = raptor_xml_element_get_name(element->parent->xml_element);
      /* Uh oh - mixed content, this element has elements too */
      raptor_parser_warning(rdf_parser, "element '%s' has mixed content.", 
                            parent_el_name->local_name);
    }
  }


  if(element->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTY_CONTENT) {
    element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LITERAL;
#ifdef RAPTOR_DEBUG_VERBOSE
    RAPTOR_DEBUG3("Content type changed to %s (%d)\n",
                  raptor_rdfxml_element_content_type_as_string(element->content_type),
                  element->content_type);
#endif
  }

  if(element->child_content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_XML_LITERAL)
    raptor_xml_writer_cdata_counted(rdf_xml_parser->xml_writer, s, len);
  else {
    raptor_stringbuffer_append_counted_string(xml_element->content_cdata_sb,
                                              s, len, 1);
    element->content_cdata_all_whitespace &= all_whitespace;
    
    /* adjust stored length */
    xml_element->content_cdata_length += len;
  }


#ifdef RAPTOR_DEBUG_CDATA
  RAPTOR_DEBUG3("Content cdata now: %d bytes\n",
                xml_element->content_cdata_length);
#endif
#ifdef RAPTOR_DEBUG_VERBOSE
  RAPTOR_DEBUG2("Ending in state %s\n", raptor_rdfxml_state_as_string(state));
#endif
}

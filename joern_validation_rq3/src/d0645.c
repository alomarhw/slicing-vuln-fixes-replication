raptor_libxml_internalSubset(void* user_data, const xmlChar *name,
                             const xmlChar *ExternalID, const xmlChar *SystemID) {
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  libxml2_internalSubset(sax2->xc, name, ExternalID, SystemID);
}

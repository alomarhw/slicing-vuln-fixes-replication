XMLRPC_CASE XMLRPC_SetDefaultIdCase(XMLRPC_CASE id_case) {
   XMLRPC_OPTIONS options = XMLRPC_GetDefaultOptions();
   options->id_case = id_case;
   return options->id_case;
}

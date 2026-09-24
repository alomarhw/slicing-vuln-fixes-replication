crm_xml_init(void)
{
    static bool init = TRUE;

    if(init) {
        init = FALSE;
        /* The default allocator XML_BUFFER_ALLOC_EXACT does far too many
         * realloc_safe()s and it can take upwards of 18 seconds (yes, seconds)
         * to dump a 28kb tree which XML_BUFFER_ALLOC_DOUBLEIT can do in
         * less than 1 second.
         */
        xmlSetBufferAllocationScheme(XML_BUFFER_ALLOC_DOUBLEIT);

        /* Populate and free the _private field when nodes are created and destroyed */
        xmlDeregisterNodeDefault(pcmkDeregisterNode);
        xmlRegisterNodeDefault(pcmkRegisterNode);

        __xml_build_schema_list();
    }
}

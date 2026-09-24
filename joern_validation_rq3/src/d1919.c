validate_with(xmlNode * xml, int method, gboolean to_logs)
{
    xmlDocPtr doc = NULL;
    gboolean valid = FALSE;
    int type = 0;
    char *file = NULL;

    if(method < 0) {
        return FALSE;
    }

    type = known_schemas[method].type;
    if(type == 0) {
        return TRUE;
    }

    CRM_CHECK(xml != NULL, return FALSE);
    doc = getDocPtr(xml);
    file = get_schema_path(known_schemas[method].name, known_schemas[method].location);

    crm_trace("Validating with: %s (type=%d)", crm_str(file), type);
    switch (type) {
        case 1:
            valid = validate_with_dtd(doc, to_logs, file);
            break;
        case 2:
            valid =
                validate_with_relaxng(doc, to_logs, file,
                                      (relaxng_ctx_cache_t **) & (known_schemas[method].cache));
            break;
        default:
            crm_err("Unknown validator type: %d", type);
            break;
    }

    free(file);
    return valid;
}

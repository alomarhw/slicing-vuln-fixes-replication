create_xml_node(xmlNode * parent, const char *name)
{
    xmlDoc *doc = NULL;
    xmlNode *node = NULL;

    if (name == NULL || name[0] == 0) {
        CRM_CHECK(name != NULL && name[0] == 0, return NULL);
        return NULL;
    }

    if (parent == NULL) {
        doc = xmlNewDoc((const xmlChar *)"1.0");
        node = xmlNewDocRawNode(doc, NULL, (const xmlChar *)name, NULL);
        xmlDocSetRootElement(doc, node);

    } else {
        doc = getDocPtr(parent);
        node = xmlNewDocRawNode(doc, NULL, (const xmlChar *)name, NULL);
        xmlAddChild(parent, node);
    }
    crm_node_created(node);
    return node;
}

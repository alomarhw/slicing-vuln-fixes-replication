COMPS_ObjRTreeData * comps_objrtree_data_create(char *key, COMPS_Object *data) {
    COMPS_ObjRTreeData * rtd;
    rtd = __comps_objrtree_data_create(key, strlen(key), data);
    return rtd;
}

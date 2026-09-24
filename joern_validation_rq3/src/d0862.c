CmpRoute(LPVOID item, LPVOID route)
{
    return memcmp(item, route, sizeof(MIB_IPFORWARD_ROW2)) == 0 ? TRUE : FALSE;
}

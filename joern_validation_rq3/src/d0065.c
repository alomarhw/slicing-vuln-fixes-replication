static int req_emerg(lua_State *L)
{
    return req_log_at(L, APLOG_EMERG);
}

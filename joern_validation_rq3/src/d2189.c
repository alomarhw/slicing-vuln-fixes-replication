alloc_trace(const char *chars, gs_ref_memory_t * imem, client_name_t cname,
            gs_memory_type_ptr_t stype, uint size, const void *ptr)
{
    if_debug7m('A', (const gs_memory_t *)imem, "[a%d%s]%s %s(%u) %s0x%lx\n",
               alloc_trace_space(imem), chars, client_name_string(cname),
               (ptr == 0 || stype == 0 ? "" :
                struct_type_name_string(stype)),
               size, (chars[1] == '+' ? "= " : ""), (ulong) ptr);
}

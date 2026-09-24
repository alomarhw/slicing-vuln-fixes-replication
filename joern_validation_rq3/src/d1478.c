static char *missing_items_in_comma_list(const char *input_item_list)
{
    if (!input_item_list)
        return NULL;

    char *item_list = xstrdup(input_item_list);
    char *result = item_list;
    char *dst = item_list;

    while (item_list[0])
    {
        char *end = strchr(item_list, ',');
        if (end) *end = '\0';
        if (!problem_data_get_item_or_NULL(g_cd, item_list))
        {
            if (dst != result)
                *dst++ = ',';
            dst = stpcpy(dst, item_list);
        }
        if (!end)
            break;
        *end = ',';
        item_list = end + 1;
    }
    if (result == dst)
    {
        free(result);
        result = NULL;
    }
    return result;
}

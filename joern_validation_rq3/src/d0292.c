wiki_show_create_page(HttpResponse *res)
{
  wiki_show_header(res, "Create New Page", FALSE);
  http_response_printf(res, "%s", CREATEFORM);
  wiki_show_footer(res);

  http_response_send(res);
  exit(0);
}

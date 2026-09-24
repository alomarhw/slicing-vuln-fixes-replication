const MenuCreateMaterial* GetFaviconMenu(Profile* profile,
                                         MenuGtk::Delegate* delegate) {
  static bool favicon_menu_built = false;
  static MenuCreateMaterial* favicon_menu;
  if (!favicon_menu_built) {
    const MenuCreateMaterial* standard_page =
        GetStandardPageMenu(profile, delegate);
    int standard_page_menu_length = 1;
    int start_offset = 0;
    for (int i = 0; standard_page[i].type != MENU_END; ++i) {
      if (standard_page[i].id == IDC_CREATE_SHORTCUTS) {
        start_offset = i + 2;
        ++i;
        continue;
      } else if (start_offset == 0) {
        NOTREACHED();
        continue;
      }

      standard_page_menu_length++;
    }
    favicon_menu = new MenuCreateMaterial[arraysize(g_favicon_menu) +
                                          standard_page_menu_length];
    memcpy(favicon_menu, g_favicon_menu,
           arraysize(g_favicon_menu) * sizeof(MenuCreateMaterial));
    memcpy(favicon_menu + arraysize(g_favicon_menu),
           standard_page + start_offset,
           (standard_page_menu_length) * sizeof(MenuCreateMaterial));
  }
  return favicon_menu;
}

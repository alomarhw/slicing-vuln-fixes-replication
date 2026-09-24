GtkWidget* CreateMenuBar(Shell* shell) {
  GtkWidget* menu_bar = gtk_menu_bar_new();
  GtkWidget* debug_menu = CreateMenu(menu_bar, "Debug");
  AddMenuEntry(debug_menu, "Show web inspector...",
               G_CALLBACK(ShowWebInspectorActivated), shell);
  return menu_bar;
}

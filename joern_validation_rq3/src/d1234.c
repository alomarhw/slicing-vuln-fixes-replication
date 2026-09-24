bool CheckMenuItem(int depth, const PP_Flash_MenuItem* item) {
  if (item->type == PP_FLASH_MENUITEM_TYPE_SUBMENU)
    return CheckMenu(depth, item->submenu);
  return true;
}

TabStripGtk::TabStripGtk(TabStripModel* model, BrowserWindowGtk* window)
    : current_unselected_width_(TabGtk::GetStandardSize().width()),
      current_selected_width_(TabGtk::GetStandardSize().width()),
      available_width_for_tabs_(-1),
      needs_resize_layout_(false),
      tab_vertical_offset_(0),
      model_(model),
      window_(window),
      theme_service_(GtkThemeService::GetFrom(model->profile())),
      weak_factory_(this),
      layout_factory_(this),
      added_as_message_loop_observer_(false),
      hover_tab_selector_(model) {
}

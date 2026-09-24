void Browser::ViewSource(TabContentsWrapper* contents,
                         const GURL& url,
                         const std::string& content_state) {
  UserMetrics::RecordAction(UserMetricsAction("ViewSource"));
  DCHECK(contents);

  TabContentsWrapper* view_source_contents = contents->Clone();
  view_source_contents->controller().PruneAllButActive();
  NavigationEntry* active_entry =
      view_source_contents->controller().GetActiveEntry();
  if (!active_entry)
    return;

  GURL view_source_url = GURL(chrome::kViewSourceScheme + std::string(":") +
      url.spec());
  active_entry->set_virtual_url(view_source_url);

  active_entry->set_content_state(
      webkit_glue::RemoveScrollOffsetFromHistoryState(content_state));

  active_entry->set_title(string16());

  if (CanSupportWindowFeature(FEATURE_TABSTRIP)) {
    int index = tab_handler_->GetTabStripModel()->
        GetIndexOfTabContents(contents);
    int add_types = TabStripModel::ADD_ACTIVE |
        TabStripModel::ADD_INHERIT_GROUP;
    tab_handler_->GetTabStripModel()->InsertTabContentsAt(index + 1,
                                                          view_source_contents,
                                                          add_types);
  } else {
    Browser* browser = Browser::CreateForType(TYPE_TABBED, profile_);

    BrowserWindow* new_window = browser->window();
    new_window->SetBounds(gfx::Rect(new_window->GetRestoredBounds().origin(),
                          window()->GetRestoredBounds().size()));

    browser->window()->Show();

    browser->AddTab(view_source_contents, PageTransition::LINK);
  }

  SessionService* session_service =
      SessionServiceFactory::GetForProfileIfExisting(profile_);
  if (session_service)
    session_service->TabRestored(view_source_contents, false);
}

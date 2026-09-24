void WebsiteSettings::PresentSiteData() {
  CookieInfoList cookie_info_list;
  const LocalSharedObjectsCounter& allowed_objects =
      tab_specific_content_settings()->allowed_local_shared_objects();
  const LocalSharedObjectsCounter& blocked_objects =
      tab_specific_content_settings()->blocked_local_shared_objects();

  WebsiteSettingsUI::CookieInfo cookie_info;
  std::string cookie_source =
      net::registry_controlled_domains::GetDomainAndRegistry(
          site_url_,
          net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
  if (cookie_source.empty())
    cookie_source = site_url_.host();
  cookie_info.cookie_source = cookie_source;
  cookie_info.allowed = allowed_objects.GetObjectCountForDomain(site_url_);
  cookie_info.blocked = blocked_objects.GetObjectCountForDomain(site_url_);
  cookie_info_list.push_back(cookie_info);

  cookie_info.cookie_source = l10n_util::GetStringUTF8(
     IDS_WEBSITE_SETTINGS_THIRD_PARTY_SITE_DATA);
  cookie_info.allowed = allowed_objects.GetObjectCount() - cookie_info.allowed;
  cookie_info.blocked = blocked_objects.GetObjectCount() - cookie_info.blocked;
  cookie_info_list.push_back(cookie_info);

  ui_->SetCookieInfo(cookie_info_list);
}

ChromeRenderMessageFilter::ChromeRenderMessageFilter(
    int render_process_id,
    Profile* profile,
    net::URLRequestContextGetter* request_context)
    : render_process_id_(render_process_id),
      profile_(profile),
      off_the_record_(profile_->IsOffTheRecord()),
      request_context_(request_context),
      extension_info_map_(
          extensions::ExtensionSystem::Get(profile)->info_map()),
      cookie_settings_(CookieSettings::Factory::GetForProfile(profile)),
      weak_ptr_factory_(ALLOW_THIS_IN_INITIALIZER_LIST(this)) {
}

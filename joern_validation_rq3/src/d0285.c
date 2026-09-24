OverscrollControllerAndroid::OverscrollControllerAndroid(
    ContentViewCoreImpl* content_view_core)
    : compositor_(content_view_core->GetWindowAndroid()->GetCompositor()),
      dpi_scale_(content_view_core->GetDpiScale()),
      enabled_(true),
      glow_effect_(CreateGlowEffect(this, dpi_scale_)),
      refresh_effect_(CreateRefreshEffect(content_view_core)),
      is_fullscreen_(false) {
  DCHECK(compositor_);
  if (refresh_effect_) {
    WebContentsImpl* web_contents =
        static_cast<WebContentsImpl*>(content_view_core->GetWebContents());
    is_fullscreen_ = web_contents->IsFullscreenForCurrentTab();
    Observe(web_contents);
  }
}

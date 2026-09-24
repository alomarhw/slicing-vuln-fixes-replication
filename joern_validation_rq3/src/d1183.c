bool RenderViewImpl::openDateTimeChooser(
    const WebKit::WebDateTimeChooserParams& params,
    WebKit::WebDateTimeChooserCompletion* completion) {
  date_time_picker_client_.reset(
      new RendererDateTimePicker(this, params, completion));
  return date_time_picker_client_->Open();
}

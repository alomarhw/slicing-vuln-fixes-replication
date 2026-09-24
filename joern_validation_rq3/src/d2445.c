void PepperVideoRenderer2D::OnViewChanged(const pp::View& view) {
  DCHECK(CalledOnValidThread());

  bool view_changed = false;

  pp::Rect pp_size = view.GetRect();
  webrtc::DesktopSize new_dips_size(pp_size.width(), pp_size.height());
  float new_dips_to_device_scale = view.GetDeviceScale();

  if (!dips_size_.equals(new_dips_size) ||
      dips_to_device_scale_ != new_dips_to_device_scale) {
    view_changed = true;
    dips_to_device_scale_ = new_dips_to_device_scale;
    dips_size_ = new_dips_size;

    dips_to_view_scale_ = 1.0f;
    view_size_ = dips_size_;

    if (!dips_size_.equals(source_size_)) {
      dips_to_view_scale_ = dips_to_device_scale_;
      view_size_.set(ceilf(dips_size_.width() * dips_to_view_scale_),
                     ceilf(dips_size_.height() * dips_to_view_scale_));
    }

    pp::Size pp_size = pp::Size(view_size_.width(), view_size_.height());
    graphics2d_ = pp::Graphics2D(instance_, pp_size, false);

    graphics2d_.SetScale(1.0f / dips_to_view_scale_);

    bool result = instance_->BindGraphics(graphics2d_);

    DCHECK(result) << "Couldn't bind the device context.";
  }

  webrtc::DesktopRect new_clip =
      view.IsVisible() ? webrtc::DesktopRect::MakeWH(
                             ceilf(pp_size.width() * dips_to_view_scale_),
                             ceilf(pp_size.height() * dips_to_view_scale_))
                       : webrtc::DesktopRect();
  if (!clip_area_.equals(new_clip)) {
    view_changed = true;

    clip_area_ = AlignRect(new_clip);
    clip_area_.IntersectWith(webrtc::DesktopRect::MakeSize(view_size_));
  }

  if (view_changed) {
    software_video_renderer_->SetOutputSizeAndClip(view_size_, clip_area_);
    AllocateBuffers();
   }
 }

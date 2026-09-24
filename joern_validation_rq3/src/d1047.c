void Instance::ConfigureProgressBar() {
  pp::ImageData background;
  std::vector<pp::ImageData> images = GetProgressBarResources(&background);
  progress_bar_.Reconfigure(background, images, device_scale_);
}

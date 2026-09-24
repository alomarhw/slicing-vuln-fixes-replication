GpuVideoDecodeAccelerator::~GpuVideoDecodeAccelerator() {
  if (stub_)
    stub_->RemoveDestructionObserver(this);
  if (video_decode_accelerator_.get())
    video_decode_accelerator_.release()->Destroy();
}

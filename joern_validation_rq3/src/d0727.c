bool LayerTreeHostImpl::HaveRootScrollLayer() const {
  return !!InnerViewportScrollLayer();
}

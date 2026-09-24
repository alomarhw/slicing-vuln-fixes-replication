void LayerTreeCoordinator::setLayerAnimatedTransform(WebLayerID id, const TransformationMatrix& transform)
{
    m_shouldSyncFrame = true;
    m_webPage->send(Messages::LayerTreeCoordinatorProxy::SetLayerAnimatedTransform(id, transform));
}

void LayerTreeHostQt::setVisibleContentsRectForPanning(const IntRect& rect, const FloatPoint& trajectoryVector)
{
    m_visibleContentsRect = rect;

    toWebGraphicsLayer(m_nonCompositedContentLayer.get())->setVisibleContentRectTrajectoryVector(trajectoryVector);

    scheduleLayerFlush();
}

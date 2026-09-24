void LayerWebKitThread::updateLayerHierarchy()
{
    m_layerCompositingThread->setSuperlayer(superlayer() ? superlayer()->m_layerCompositingThread.get() : 0);

    Vector<RefPtr<LayerCompositingThread> > sublayers;
    size_t listSize = m_overlays.size();
    for (size_t i = 0; i < listSize; ++i)
        sublayers.append(m_overlays[i]->m_layerCompositingThread.get());
    listSize = m_sublayers.size();
    for (size_t i = 0; i < listSize; ++i)
        sublayers.append(m_sublayers[i]->m_layerCompositingThread.get());
    m_layerCompositingThread->setSublayers(sublayers);
}

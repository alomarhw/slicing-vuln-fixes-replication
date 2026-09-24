void Document::setVisuallyOrdered()
{
    m_visuallyOrdered = true;
    if (renderer())
        renderer()->style()->setRTLOrdering(VisualOrder);
}

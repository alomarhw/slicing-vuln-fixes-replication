FloatRect WebPagePrivate::mapFromTransformedFloatRect(const FloatRect& rect) const
{
    return m_transformationMatrix->inverse().mapRect(rect);
}

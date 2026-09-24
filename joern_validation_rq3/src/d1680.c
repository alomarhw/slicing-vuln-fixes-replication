bool Range::boundaryPointsValid() const
{
    ExceptionCode ec = 0;
    return m_start.container() && compareBoundaryPoints(m_start, m_end, ec) <= 0 && !ec;
}

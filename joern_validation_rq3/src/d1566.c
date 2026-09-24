inline void LineWidth::updateAvailableWidth(LayoutUnit replacedHeight)
{
    LayoutUnit height = m_block->logicalHeight();
    LayoutUnit logicalHeight = logicalHeightForLine(m_block, m_isFirstLine, replacedHeight);
    m_left = m_block->logicalLeftOffsetForLine(height, shouldIndentText(), logicalHeight);
    m_right = m_block->logicalRightOffsetForLine(height, shouldIndentText(), logicalHeight);

    if (m_segment) {
        m_left = max<float>(m_segment->logicalLeft, m_left);
        m_right = min<float>(m_segment->logicalRight, m_right);
    }

    computeAvailableWidthFromLeftAndRight();
}

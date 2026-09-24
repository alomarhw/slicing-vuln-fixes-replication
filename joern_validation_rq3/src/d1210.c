bool RenderBlock::avoidsFloats() const
{
    return RenderBox::avoidsFloats() || !style()->hasAutoColumnCount() || !style()->hasAutoColumnWidth();
}

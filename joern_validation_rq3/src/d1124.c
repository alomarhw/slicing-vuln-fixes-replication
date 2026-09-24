static SelectionInFlatTree ExtendSelectionAsDirectional(
    const PositionInFlatTree& position,
    const VisibleSelectionInFlatTree& selection,
    TextGranularity granularity) {
  DCHECK(!selection.IsNone());
  DCHECK(position.IsNotNull());
  const PositionInFlatTree& start = selection.Start();
  const PositionInFlatTree& end = selection.End();
  const PositionInFlatTree& base = selection.IsBaseFirst() ? start : end;
  if (position < base) {
    const PositionInFlatTree& new_start = ComputeStartRespectingGranularity(
        PositionInFlatTreeWithAffinity(position), granularity);
    const PositionInFlatTree& new_end =
        selection.IsBaseFirst()
            ? ComputeEndRespectingGranularity(
                  new_start, PositionInFlatTreeWithAffinity(start), granularity)
            : end;
    return SelectionInFlatTree::Builder()
        .SetBaseAndExtent(new_end, new_start)
        .Build();
  }

  const PositionInFlatTree& new_start =
      selection.IsBaseFirst()
          ? start
          : ComputeStartFromEndForExtendForward(end, granularity);
  const PositionInFlatTree& new_end = ComputeEndRespectingGranularity(
      new_start, PositionInFlatTreeWithAffinity(position), granularity);
  return SelectionInFlatTree::Builder()
      .SetBaseAndExtent(new_start, new_end)
      .Build();
}

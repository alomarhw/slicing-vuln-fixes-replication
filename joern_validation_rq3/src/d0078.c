void TabStripGtk::OnSizeAllocate(GtkWidget* widget, GtkAllocation* allocation) {
  TRACE_EVENT0("ui::gtk", "TabStripGtk::OnSizeAllocate");

  gfx::Rect bounds = gfx::Rect(allocation->x, allocation->y,
      allocation->width, allocation->height);

  if (bounds_ == bounds)
    return;

  SetBounds(bounds);

  if (GetTabCount() == 0)
    return;

  if (GetTabCount() == 1) {
    Layout();
  } else if (!layout_factory_.HasWeakPtrs()) {
    MessageLoop::current()->PostDelayedTask(
        FROM_HERE,
        base::Bind(&TabStripGtk::Layout, layout_factory_.GetWeakPtr()),
        base::TimeDelta::FromMilliseconds(kLayoutAfterSizeAllocateMs));
  }
}

Address NormalPageArena::LazySweepPages(size_t allocation_size,
                                        size_t gc_info_index) {
  DCHECK(!HasCurrentAllocationArea());
  AutoReset<bool> is_lazy_sweeping(&is_lazy_sweeping_, true);
  Address result = nullptr;
  while (!SweepingCompleted()) {
    BasePage* page = first_unswept_page_;
    if (page->IsEmpty()) {
      page->Unlink(&first_unswept_page_);
      page->RemoveFromHeap();
    } else {
      page->Sweep();
      page->Unlink(&first_unswept_page_);
      page->Link(&first_page_);
      page->MarkAsSwept();

      result = AllocateFromFreeList(allocation_size, gc_info_index);
      if (result)
        break;
    }
  }
  return result;
}

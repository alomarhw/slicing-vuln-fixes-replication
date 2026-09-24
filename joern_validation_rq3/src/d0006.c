PrintingContext::Result PrintingContext::PageDone() {
  if (abort_printing_)
    return CANCEL;
  DCHECK(in_print_job_);

  if (EndPage(context_) <= 0)
    return OnError();
  return OK;
}

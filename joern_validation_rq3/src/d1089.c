int CALLBACK SelectFileDialogImpl::BrowseCallbackProc(HWND window,
                                                      UINT message,
                                                      LPARAM parameter,
                                                      LPARAM data) {
  if (message == BFFM_INITIALIZED) {
    SendMessage(window, BFFM_SETSELECTION, TRUE, (LPARAM)data);
  }
  return 0;
}

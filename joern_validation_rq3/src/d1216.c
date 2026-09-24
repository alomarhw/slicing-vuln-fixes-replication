void PrintPreviewDialogController::OnRendererProcessClosed(
    content::RenderProcessHost* rph) {
  std::vector<WebContents*> closed_initiators;
  std::vector<WebContents*> closed_preview_dialogs;
  for (PrintPreviewDialogMap::iterator iter = preview_dialog_map_.begin();
       iter != preview_dialog_map_.end(); ++iter) {
    WebContents* preview_dialog = iter->first;
    WebContents* initiator = iter->second;
    if (preview_dialog->GetMainFrame()->GetProcess() == rph) {
      closed_preview_dialogs.push_back(preview_dialog);
    } else if (initiator && initiator->GetMainFrame()->GetProcess() == rph) {
      closed_initiators.push_back(initiator);
    }
  }

  for (size_t i = 0; i < closed_preview_dialogs.size(); ++i) {
    RemovePreviewDialog(closed_preview_dialogs[i]);
    if (content::WebUI* web_ui = closed_preview_dialogs[i]->GetWebUI()) {
      PrintPreviewUI* print_preview_ui =
          static_cast<PrintPreviewUI*>(web_ui->GetController());
      if (print_preview_ui)
        print_preview_ui->OnPrintPreviewDialogClosed();
    }
  }

  for (size_t i = 0; i < closed_initiators.size(); ++i)
    RemoveInitiator(closed_initiators[i]);
}

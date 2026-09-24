void Document::TasksWereSuspended() {
  GetScriptRunner()->Suspend();

  if (parser_)
    parser_->SuspendScheduledTasks();
  if (scripted_animation_controller_)
    scripted_animation_controller_->Suspend();
}

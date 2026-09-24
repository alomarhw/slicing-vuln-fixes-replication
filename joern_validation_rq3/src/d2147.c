void Document::suspendScheduledTasks(ActiveDOMObject::ReasonForSuspension reason)
{
    ASSERT(!m_scheduledTasksAreSuspended);

    suspendScriptedAnimationControllerCallbacks();
    suspendActiveDOMObjects(reason);
    scriptRunner()->suspend();
    m_pendingTasksTimer.stop();
    if (m_parser)
        m_parser->suspendScheduledTasks();

    m_scheduledTasksAreSuspended = true;
}

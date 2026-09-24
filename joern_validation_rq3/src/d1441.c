void SpeechRecognitionManagerImpl::DispatchEvent(int session_id,
                                                 FSMEvent event) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);

  if (!SessionExists(session_id))
    return;

  Session* session = GetSession(session_id);
  FSMState session_state = GetSessionState(session_id);
  DCHECK_LE(session_state, SESSION_STATE_MAX_VALUE);
  DCHECK_LE(event, EVENT_MAX_VALUE);

  DCHECK(!is_dispatching_event_);
  is_dispatching_event_ = true;
  ExecuteTransitionAndGetNextState(session, session_state, event);
  is_dispatching_event_ = false;
}

void JingleSessionManager::OnSessionCreate(
    cricket::Session* cricket_session, bool incoming) {
  DCHECK(CalledOnValidThread());

  cricket_session->set_allow_local_ips(allow_local_ips_);

  if (incoming) {
    DCHECK(!certificate_.empty());
    DCHECK(private_key_.get());

    JingleSession* jingle_session = JingleSession::CreateServerSession(
        this, certificate_, private_key_.get());
    sessions_.push_back(jingle_session);
    jingle_session->Init(cricket_session);
  }
}

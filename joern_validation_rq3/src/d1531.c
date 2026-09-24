bool CreateSessionToken(uint32 session_id, ScopedHandle* token_out) {
  ScopedHandle session_token;
  DWORD desired_access = TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID |
                         TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_QUERY;
  if (!CopyProcessToken(desired_access, &session_token)) {
    return false;
  }

  ScopedHandle privileged_token;
  if (!CreatePrivilegedToken(&privileged_token)) {
    return false;
  }
  if (!ImpersonateLoggedOnUser(privileged_token)) {
    LOG_GETLASTERROR(ERROR) <<
        "Failed to impersonate the privileged token";
    return false;
  }

  DWORD new_session_id = session_id;
  if (!SetTokenInformation(session_token,
                           TokenSessionId,
                           &new_session_id,
                           sizeof(new_session_id))) {
    LOG_GETLASTERROR(ERROR) << "Failed to change session ID of a token";

    CHECK(RevertToSelf());
    return false;
  }

  CHECK(RevertToSelf());

  *token_out = session_token.Pass();
  return true;
}

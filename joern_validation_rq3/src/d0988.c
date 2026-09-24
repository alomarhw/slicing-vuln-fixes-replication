entry_guard_failed(circuit_guard_state_t **guard_state_p)
{
  if (BUG(*guard_state_p == NULL))
    return;

  entry_guard_t *guard = entry_guard_handle_get((*guard_state_p)->guard);
  if (! guard || BUG(guard->in_selection == NULL))
    return;

  entry_guards_note_guard_failure(guard->in_selection, guard);

  (*guard_state_p)->state = GUARD_CIRC_STATE_DEAD;
  (*guard_state_p)->state_set_at = approx_time();
}

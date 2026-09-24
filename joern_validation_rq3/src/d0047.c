static TPM_RC AddSession( SESSION_LIST_ENTRY **sessionEntry )
{
    SESSION_LIST_ENTRY **newEntry;

    for( newEntry = &local_sessions_list; *newEntry != 0; *newEntry = ( (SESSION_LIST_ENTRY *)*newEntry)->nextEntry )
        ;

    *newEntry = malloc( sizeof( SESSION_LIST_ENTRY ) );
    if( *newEntry != 0 )
    {
        *sessionEntry = *newEntry;
        (*sessionEntry)->nextEntry = 0;
        local_session_entries_used++;
        return TPM_RC_SUCCESS;
    }
    else
    {
        return TSS2_APP_RC_SESSION_SLOT_NOT_FOUND;
    }
}

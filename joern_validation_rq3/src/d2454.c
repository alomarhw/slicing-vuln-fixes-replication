fast_unwrap_error(krb5_context context, struct fast_state *state, KRB_ERROR *error)
{
    if (state->armor_crypto == NULL)
	return check_fast(context, state);

    return 0;
}

int ossl_statem_accept(SSL *s)
{
    return state_machine(s, 1);
}

void rxrpc_remove_user_ID(struct rxrpc_sock *rx, struct rxrpc_call *call)
{
	_debug("RELEASE CALL %d", call->debug_id);

	if (test_bit(RXRPC_CALL_HAS_USERID, &call->flags)) {
		write_lock_bh(&rx->call_lock);
		rb_erase(&call->sock_node, &call->socket->calls);
		clear_bit(RXRPC_CALL_HAS_USERID, &call->flags);
		write_unlock_bh(&rx->call_lock);
	}

	read_lock_bh(&call->state_lock);
	if (!test_bit(RXRPC_CALL_RELEASED, &call->flags) &&
	    !test_and_set_bit(RXRPC_CALL_RELEASE, &call->events))
		rxrpc_queue_call(call);
	read_unlock_bh(&call->state_lock);
}

void dtls1_free(SSL *s)
	{
	ssl3_free(s);

	dtls1_clear_queues(s);

    pqueue_free(s->d1->unprocessed_rcds.q);
    pqueue_free(s->d1->processed_rcds.q);
    pqueue_free(s->d1->buffered_messages);
	pqueue_free(s->d1->sent_messages);
	pqueue_free(s->d1->buffered_app_data.q);

	OPENSSL_free(s->d1);
	s->d1 = NULL;
	}

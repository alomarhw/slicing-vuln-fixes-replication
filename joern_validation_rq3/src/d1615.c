static int send_hand_shake_reply(nw_ses *ses, char *protocol, const char *key)
{
    unsigned char hash[20];
    sds data = sdsnew(key);
    data = sdscat(data, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    SHA1((const unsigned char *)data, sdslen(data), hash);
    sdsfree(data);

    sds b4message;
    base64_encode(hash, sizeof(hash), &b4message);

    http_response_t *response = http_response_new();
    http_response_set_header(response, "Upgrade", "websocket");
    http_response_set_header(response, "Connection", "Upgrade");
    http_response_set_header(response, "Sec-WebSocket-Accept", b4message);
    if (protocol) {
        http_response_set_header(response, "Sec-WebSocket-Protocol", protocol);
    }
    response->status = 101;

    sds message = http_response_encode(response);
    nw_ses_send(ses, message, sdslen(message));

    sdsfree(message);
    sdsfree(b4message);

    return 0;
}

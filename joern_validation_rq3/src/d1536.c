void CastStreamingNativeHandler::CallCreateCallback(
    scoped_ptr<CastRtpStream> stream1,
    scoped_ptr<CastRtpStream> stream2,
    scoped_ptr<CastUdpTransport> udp_transport) {
  v8::Isolate* isolate = context()->isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(context()->v8_context());

  v8::Local<v8::Value> callback_args[3];
  callback_args[0] = v8::Null(isolate);
  callback_args[1] = v8::Null(isolate);

  if (stream1) {
    const int stream1_id = last_transport_id_++;
    callback_args[0] = v8::Integer::New(isolate, stream1_id);
    rtp_stream_map_[stream1_id] =
        linked_ptr<CastRtpStream>(stream1.release());
  }
  if (stream2) {
    const int stream2_id = last_transport_id_++;
    callback_args[1] = v8::Integer::New(isolate, stream2_id);
    rtp_stream_map_[stream2_id] =
        linked_ptr<CastRtpStream>(stream2.release());
  }
  const int udp_id = last_transport_id_++;
  udp_transport_map_[udp_id] =
      linked_ptr<CastUdpTransport>(udp_transport.release());
  callback_args[2] = v8::Integer::New(isolate, udp_id);
  context()->CallFunction(
      v8::Local<v8::Function>::New(isolate, create_callback_), 3,
      callback_args);
  create_callback_.Reset();
}

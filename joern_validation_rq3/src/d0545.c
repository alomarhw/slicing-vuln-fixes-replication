void ExtensionDevToolsClientHost::SendMessageToBackend(
    SendCommandDebuggerFunction* function,
    const std::string& method,
    Value* params) {
  DictionaryValue protocol_request;
  int request_id = ++last_request_id_;
  pending_requests_[request_id] = function;
  protocol_request.SetInteger("id", request_id);
  protocol_request.SetString("method", method);
  if (params)
    protocol_request.Set("params", params->DeepCopy());

  std::string json_args;
  base::JSONWriter::Write(&protocol_request, false, &json_args);
  DevToolsManager::GetInstance()->DispatchOnInspectorBackend(this, json_args);
}

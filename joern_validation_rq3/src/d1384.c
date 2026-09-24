Response InspectorPageAgent::addScriptToEvaluateOnLoad(const String& source,
                                                       String* identifier) {
  protocol::DictionaryValue* scripts =
      state_->getObject(PageAgentState::kPageAgentScriptsToEvaluateOnLoad);
  if (!scripts) {
    std::unique_ptr<protocol::DictionaryValue> new_scripts =
        protocol::DictionaryValue::create();
    scripts = new_scripts.get();
    state_->setObject(PageAgentState::kPageAgentScriptsToEvaluateOnLoad,
                      std::move(new_scripts));
  }
  do {
    *identifier = String::Number(++last_script_identifier_);
  } while (scripts->get(*identifier));
  scripts->setString(*identifier, source);
  return Response::OK();
}

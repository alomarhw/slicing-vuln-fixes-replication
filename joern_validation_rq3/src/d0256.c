void ChromotingInstance::HandleOnPinFetched(const base::DictionaryValue& data) {
  std::string pin;
  if (!data.GetString("pin", &pin)) {
    LOG(ERROR) << "Invalid onPinFetched.";
    return;
  }
  if (!secret_fetched_callback_.is_null()) {
    secret_fetched_callback_.Run(pin);
    secret_fetched_callback_.Reset();
  } else {
    LOG(WARNING) << "Ignored OnPinFetched received without a pending fetch.";
  }
}

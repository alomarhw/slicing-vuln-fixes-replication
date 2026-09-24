Value* ChromeNetworkDelegate::HistoricNetworkStatsInfoToValue() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  PrefService* prefs = g_browser_process->local_state();
  int64 total_received = prefs->GetInt64(prefs::kHttpReceivedContentLength);
  int64 total_original = prefs->GetInt64(prefs::kHttpOriginalContentLength);

  DictionaryValue* dict = new DictionaryValue();
  dict->SetString("historic_received_content_length",
                  base::Int64ToString(total_received));
  dict->SetString("historic_original_content_length",
                  base::Int64ToString(total_original));
  return dict;
}

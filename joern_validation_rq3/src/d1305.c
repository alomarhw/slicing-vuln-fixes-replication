void HttpResponseHeaders::GetNormalizedHeaders(std::string* output) const {
  output->assign(raw_headers_.c_str());

  typedef base::hash_map<std::string, size_t> HeadersMap;
  HeadersMap headers_map;
  HeadersMap::iterator iter = headers_map.end();

  std::vector<std::string> headers;

  for (size_t i = 0; i < parsed_.size(); ++i) {
    DCHECK(!parsed_[i].is_continuation());

    std::string name(parsed_[i].name_begin, parsed_[i].name_end);
    std::string lower_name = StringToLowerASCII(name);

    iter = headers_map.find(lower_name);
    if (iter == headers_map.end()) {
      iter = headers_map.insert(
          HeadersMap::value_type(lower_name, headers.size())).first;
      headers.push_back(name + ": ");
    } else {
      headers[iter->second].append(", ");
    }

    std::string::const_iterator value_begin = parsed_[i].value_begin;
    std::string::const_iterator value_end = parsed_[i].value_end;
    while (++i < parsed_.size() && parsed_[i].is_continuation())
      value_end = parsed_[i].value_end;
    --i;

    headers[iter->second].append(value_begin, value_end);
  }

  for (size_t i = 0; i < headers.size(); ++i) {
    output->push_back('\n');
    output->append(headers[i]);
  }

  output->push_back('\n');
}

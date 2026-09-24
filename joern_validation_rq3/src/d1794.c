  discourse_context::ClientDiscourseContext GetDiscourseContextFromRequest() {
    discourse_context::ClientDiscourseContext cdc;
    net::HttpRequestHeaders fetch_headers;
    fetcher()->GetExtraRequestHeaders(&fetch_headers);
    if (fetch_headers.HasHeader(kDiscourseContextHeaderName)) {
      std::string actual_header_value;
      fetch_headers.GetHeader(kDiscourseContextHeaderName,
                              &actual_header_value);

      std::string unescaped_header = actual_header_value;
      std::replace(unescaped_header.begin(), unescaped_header.end(), '-', '+');
      std::replace(unescaped_header.begin(), unescaped_header.end(), '_', '/');

      std::string decoded_header;
      if (base::Base64Decode(unescaped_header, &decoded_header)) {
        cdc.ParseFromString(decoded_header);
      }
    }
    return cdc;
  }

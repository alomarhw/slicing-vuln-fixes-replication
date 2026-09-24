String XSSAuditor::SnippetFromAttribute(const FilterTokenRequest& request,
                                        const HTMLToken::Attribute& attribute) {
  int name_start = attribute.NameRange().start - request.token.StartIndex();
  int value_start = attribute.ValueRange().start - request.token.StartIndex();
  int value_end = attribute.ValueRange().end - request.token.StartIndex();
  int length = value_end - name_start;
  if (value_start == value_end)
    length += 1;
  return request.source_tracker.SourceForToken(request.token)
      .Substring(name_start, length);
}

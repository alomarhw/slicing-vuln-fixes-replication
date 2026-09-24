const KURL Document::SiteForCookies() const {

  if (IsHTMLImport())
    return ImportsController()->Master()->SiteForCookies();

  if (!GetFrame())
    return NullURL();

  Frame& top = GetFrame()->Tree().Top();
  KURL top_document_url;
  auto* top_local_frame = DynamicTo<LocalFrame>(&top);
  if (top_local_frame) {
    top_document_url = top_local_frame->GetDocument()->Url();
  } else {
    const SecurityOrigin* origin =
        top.GetSecurityContext()->GetSecurityOrigin();
    if (origin)
      top_document_url = KURL(NullURL(), origin->ToString());
    else
      top_document_url = NullURL();
  }

  if (SchemeRegistry::ShouldTreatURLSchemeAsFirstPartyWhenTopLevel(
          top_document_url.Protocol()))
    return top_document_url;

  base::Optional<OriginAccessEntry> remote_entry;
  if (!top_local_frame) {
    remote_entry.emplace(
        top_document_url.Protocol(), top_document_url.Host(),
        network::mojom::CorsOriginAccessMatchMode::kAllowRegisterableDomains);
  }
  const OriginAccessEntry& access_entry =
      remote_entry ? *remote_entry
                   : top_local_frame->GetDocument()->AccessEntryFromURL();

  const Frame* current_frame = GetFrame();
  while (current_frame) {
    auto is_srcdoc = [](const Frame* frame) {
      const auto* local_frame = DynamicTo<LocalFrame>(frame);
      return local_frame && local_frame->GetDocument()->IsSrcdocDocument();
    };
    while (is_srcdoc(current_frame))
      current_frame = current_frame->Tree().Parent();
    DCHECK(current_frame);

    if (access_entry.MatchesDomain(
            *current_frame->GetSecurityContext()->GetSecurityOrigin()) ==
        network::cors::OriginAccessEntry::kDoesNotMatchOrigin) {
      return NullURL();
    }

    current_frame = current_frame->Tree().Parent();
  }

  return top_document_url;
}

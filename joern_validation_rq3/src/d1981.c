void Document::setTitle(const String& title) {
  if (!title_element_) {
    if (IsHTMLDocument() || IsXHTMLDocument()) {
      HTMLElement* head_element = head();
      if (!head_element)
        return;
      title_element_ = HTMLTitleElement::Create(*this);
      head_element->AppendChild(title_element_.Get());
    } else if (IsSVGDocument()) {
      Element* element = documentElement();
      if (!IsSVGSVGElement(element))
        return;
      title_element_ = SVGTitleElement::Create(*this);
      element->InsertBefore(title_element_.Get(), element->firstChild());
    }
  } else {
    if (!IsHTMLDocument() && !IsXHTMLDocument() && !IsSVGDocument())
      title_element_ = nullptr;
  }

  if (auto* html_title = ToHTMLTitleElementOrNull(title_element_))
    html_title->setText(title);
  else if (auto* svg_title = ToSVGTitleElementOrNull(title_element_))
    svg_title->SetText(title);
  else
    UpdateTitle(title);
}

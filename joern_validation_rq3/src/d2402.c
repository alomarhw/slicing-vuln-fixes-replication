void HTMLMediaElement::ensureMediaControls() {
  if (mediaControls())
    return;

  ShadowRoot& shadowRoot = ensureUserAgentShadowRoot();
  m_mediaControls = MediaControls::create(*this, shadowRoot);

  assertShadowRootChildren(shadowRoot);
}

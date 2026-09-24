bool BrowserPluginGuest::ViewTakeFocus(bool reverse) {
  SendMessageToEmbedder(
      new BrowserPluginMsg_AdvanceFocus(embedder_routing_id(),
                                        instance_id(),
                                        reverse));
  return true;
}

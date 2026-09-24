 void AXObjectCacheImpl::postPlatformNotification(AXObject* obj,
                                                 AXNotification notification) {
  if (!obj || !obj->getDocument() || !obj->documentFrameView() ||
      !obj->documentFrameView()->frame().page())
    return;

  ChromeClient& client =
      obj->getDocument()->axObjectCacheOwner().page()->chromeClient();
  client.postAccessibilityNotification(obj, notification);
}

 GuestViewBase* GuestViewBase::FromWebContents(const WebContents* web_contents) {
   WebContentsGuestViewMap* guest_map = webcontents_guestview_map.Pointer();
  auto it = guest_map->find(web_contents);
  return it == guest_map->end() ? nullptr : it->second;
}

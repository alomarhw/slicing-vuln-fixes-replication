BookmarkDrag::BookmarkDrag(Profile* profile,
                           const std::vector<const BookmarkNode*>& nodes)
    : CustomDrag(NULL,
                 bookmark_utils::GetCodeMask(false),
                 kBookmarkDragAction),
      profile_(profile),
      nodes_(nodes) {
}

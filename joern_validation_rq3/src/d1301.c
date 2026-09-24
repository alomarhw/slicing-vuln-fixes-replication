nautilus_file_invalidate_count_and_mime_list (NautilusFile *file)
{
    NautilusFileAttributes attributes;

    attributes = NAUTILUS_FILE_ATTRIBUTE_DIRECTORY_ITEM_COUNT |
                 NAUTILUS_FILE_ATTRIBUTE_DIRECTORY_ITEM_MIME_TYPES;

    nautilus_file_invalidate_attributes (file, attributes);
}

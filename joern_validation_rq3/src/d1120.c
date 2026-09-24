static int PNMComment(Image *image,CommentInfo *comment_info,
  ExceptionInfo *exception)
{
  int
    c;

  register char
    *p;

  /*
    Read comment.
  */
  p=comment_info->comment+strlen(comment_info->comment);
  for (c='#'; (c != EOF) && (c != (int) '\n') && (c != (int) '\r'); p++)
  {
    if ((size_t) (p-comment_info->comment+1) >= comment_info->extent)
      {
        comment_info->extent<<=1;
        comment_info->comment=(char *) ResizeQuantumMemory(
          comment_info->comment,comment_info->extent,
          sizeof(*comment_info->comment));
        if (comment_info->comment == (char *) NULL)
          return(-1);
        p=comment_info->comment+strlen(comment_info->comment);
      }
    c=ReadBlobByte(image);
    if (c != EOF)
      {
        *p=(char) c;
        *(p+1)='\0';
      }
  }
  return(c);
}

static inline unsigned int ReadProfileMSBLong(unsigned char **p,size_t *length)
{
  unsigned int
    value;

  if (*length < 4)
    return(0);
  value=ReadProfileLong(MSBEndian,*p);
  (*length)-=4;
  *p+=4;
  return(value);
}

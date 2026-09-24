int yr_re_match(
    RE* re,
    const char* target)
{
  int result;

  yr_re_exec(
      re->code,
      (uint8_t*) target,
      strlen(target),
      0,
      re->flags | RE_FLAGS_SCAN,
      NULL,
      NULL,
      &result);

  return result;
}

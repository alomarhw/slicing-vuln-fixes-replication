raptor_object_options_get_option(raptor_object_options* options,
                                 raptor_option option,
                                 char** string_p, int* integer_p)
{
  if(!raptor_option_is_valid_for_area(option, options->area))
    return 1;
  
  if(raptor_option_value_is_numeric(option)) {
    /* numeric options */
    int value = options->options[(int)option].integer;
    if(integer_p)
      *integer_p = value;
  } else {
    /* non-numeric options */
    char* string = options->options[(int)option].string;
    if(string_p)
      *string_p = string;
  }
  
  return 0;
}

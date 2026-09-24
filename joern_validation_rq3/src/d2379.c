set_home_var ()
{
  SHELL_VAR *temp_var;

  temp_var = find_variable ("HOME");
  if (temp_var == 0)
    temp_var = bind_variable ("HOME", sh_get_home_dir (), 0);
#if 0
  VSETATTR (temp_var, att_exported);
#endif
}

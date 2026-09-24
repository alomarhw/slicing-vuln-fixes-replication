mrb_mod_s_nesting(mrb_state *mrb, mrb_value mod)
{
  struct RProc *proc;
  mrb_value ary;
  struct RClass *c = NULL;

  mrb_get_args(mrb, "");
  ary = mrb_ary_new(mrb);
  proc = mrb->c->ci[-1].proc;   /* callee proc */
  mrb_assert(!MRB_PROC_CFUNC_P(proc));
  while (proc) {
    if (MRB_PROC_SCOPE_P(proc)) {
      struct RClass *c2 = MRB_PROC_TARGET_CLASS(proc);

      if (c2 != c) {
        c = c2;
        mrb_ary_push(mrb, ary, mrb_obj_value(c));
      }
    }
    proc = proc->upper;
  }
  return ary;
}

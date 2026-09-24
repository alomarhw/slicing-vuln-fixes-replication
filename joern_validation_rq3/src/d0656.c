ops_make_string_pool(regex_t* reg)
{
  int i;
  int len;
  int size;
  UChar* pool;
  UChar* curr;

  size = ops_calc_size_of_string_pool(reg);
  if (size <= 0) {
    return 0;
  }

  curr = pool = (UChar* )xmalloc((size_t )size);
  CHECK_NULL_RETURN_MEMERR(pool);

  for (i = 0; i < (int )reg->ops_used; i++) {
    enum OpCode opcode;
    Operation* op;

    op = reg->ops + i;
#ifdef USE_DIRECT_THREADED_CODE
    opcode = *(reg->ocs + i);
#else
    opcode = op->opcode;
#endif

    switch (opcode) {
    case OP_EXACTMBN:
      len = op->exact_len_n.len * op->exact_len_n.n;
      xmemcpy(curr, op->exact_len_n.s, len);
      xfree(op->exact_len_n.s);
      op->exact_len_n.s = curr;
      curr += len;
      break;
    case OP_EXACTN:
    case OP_EXACTN_IC:
      len = op->exact_n.n;
    copy:
      xmemcpy(curr, op->exact_n.s, len);
      xfree(op->exact_n.s);
      op->exact_n.s = curr;
      curr += len;
      break;
    case OP_EXACTMB2N:
      len = op->exact_n.n * 2;
      goto copy;
      break;
    case OP_EXACTMB3N:
      len = op->exact_n.n * 3;
      goto copy;
      break;

    default:
      break;
    }
  }

  reg->string_pool     = pool;
  reg->string_pool_end = pool + size;
  return 0;
}

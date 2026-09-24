void ParamTraits<IPC::ChannelHandle>::Write(Message* m, const param_type& p) {
#if defined(OS_WIN)
  DCHECK(p.pipe.handle == NULL);
#endif  // defined (OS_WIN)
  WriteParam(m, p.name);
#if defined(OS_POSIX)
  WriteParam(m, p.socket);
#endif
}

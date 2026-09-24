static pid_t name2pid(char *name){
  pid_t pid=-1;
  int lokke;
  int num_procs=0;
  struct das_proclist *proclist=get_proclist(&num_procs);
    
  for(lokke=0;lokke<num_procs;lokke++){
    glibtop_proc_state state;
    glibtop_get_proc_state(&state,proclist[lokke].pid);
    if(!strcmp(state.cmd,name)){
      pid=proclist[lokke].pid;
      break;
    }
  }
  free(proclist);
  return pid;
}

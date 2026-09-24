local int nprocs(int n)
{
#  ifdef _SC_NPROCESSORS_ONLN
    n = (int)sysconf(_SC_NPROCESSORS_ONLN);
#  else
#    ifdef _SC_NPROC_ONLN
    n = (int)sysconf(_SC_NPROC_ONLN);
#    else
#      ifdef __hpux
    struct pst_dynamic psd;

    if (pstat_getdynamic(&psd, sizeof(psd), (size_t)1, 0) != -1)
        n = psd.psd_proc_cnt;
#      endif
#    endif
#  endif
    return n;
}

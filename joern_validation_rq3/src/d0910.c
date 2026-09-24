hook_process_run (struct t_hook *hook_process)
{
    int pipe_stdout[2], pipe_stderr[2], timeout, max_calls;
    long interval;
    pid_t pid;
    
    /* create pipe for child process (stdout) */
    if (pipe (pipe_stdout) < 0)
    {
        (void) (HOOK_PROCESS(hook_process, callback))
            (hook_process->callback_data,
             HOOK_PROCESS(hook_process, command),
             WEECHAT_HOOK_PROCESS_ERROR,
             NULL, NULL);
        unhook (hook_process);
        return;
    }
    if (pipe (pipe_stderr) < 0)
    {
        close (pipe_stdout[0]);
        close (pipe_stdout[1]);
        (void) (HOOK_PROCESS(hook_process, callback))
            (hook_process->callback_data,
             HOOK_PROCESS(hook_process, command),
             WEECHAT_HOOK_PROCESS_ERROR,
             NULL, NULL);
        unhook (hook_process);
        return;
    }
    
    HOOK_PROCESS(hook_process, child_read[HOOK_PROCESS_STDOUT]) = pipe_stdout[0];
    HOOK_PROCESS(hook_process, child_write[HOOK_PROCESS_STDOUT]) = pipe_stdout[1];
    
    HOOK_PROCESS(hook_process, child_read[HOOK_PROCESS_STDERR]) = pipe_stderr[0];
    HOOK_PROCESS(hook_process, child_write[HOOK_PROCESS_STDERR]) = pipe_stderr[1];
    
    switch (pid = fork ())
    {
        /* fork failed */
        case -1:
            (void) (HOOK_PROCESS(hook_process, callback))
                (hook_process->callback_data,
                 HOOK_PROCESS(hook_process, command),
                 WEECHAT_HOOK_PROCESS_ERROR,
                 NULL, NULL);
            unhook (hook_process);
            return;
        /* child process */
        case 0:
            setuid (getuid ());
            hook_process_child (hook_process);
            /* never executed */
            _exit (EXIT_SUCCESS);
            break;
    }
    /* parent process */
    HOOK_PROCESS(hook_process, child_pid) = pid;
    close (HOOK_PROCESS(hook_process, child_write[HOOK_PROCESS_STDOUT]));
    HOOK_PROCESS(hook_process, child_write[HOOK_PROCESS_STDOUT]) = -1;
    close (HOOK_PROCESS(hook_process, child_write[HOOK_PROCESS_STDERR]));
    HOOK_PROCESS(hook_process, child_write[HOOK_PROCESS_STDERR]) = -1;
    
    HOOK_PROCESS(hook_process, hook_fd[HOOK_PROCESS_STDOUT]) =
        hook_fd (hook_process->plugin,
                 HOOK_PROCESS(hook_process, child_read[HOOK_PROCESS_STDOUT]),
                 1, 0, 0,
                 &hook_process_child_read_stdout_cb,
                 hook_process);
    
    HOOK_PROCESS(hook_process, hook_fd[HOOK_PROCESS_STDERR]) =
        hook_fd (hook_process->plugin,
                 HOOK_PROCESS(hook_process, child_read[HOOK_PROCESS_STDERR]),
                 1, 0, 0,
                 &hook_process_child_read_stderr_cb,
                 hook_process);
    
    timeout = HOOK_PROCESS(hook_process, timeout);
    interval = 100;
    max_calls = 0;
    if (timeout > 0)
    {
        if (timeout <= 100)
        {
            interval = timeout;
            max_calls = 1;
        }
        else
        {
            interval = 100;
            max_calls = timeout / 100;
            if (timeout % 100 == 0)
                max_calls++;
        }
    }
    HOOK_PROCESS(hook_process, hook_timer) = hook_timer (hook_process->plugin,
                                                         interval, 0, max_calls,
                                                         &hook_process_timer_cb,
                                                         hook_process);
}

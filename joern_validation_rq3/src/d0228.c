int main(int argc, char** argv) {
#if defined(OS_MACOSX)
  base::mac::ScopedNSAutoreleasePool pool;
#endif

  CommandLine::Init(argc, argv);

  base::AtExitManager exit_manager;

  if (CommandLine::ForCurrentProcess()->HasSwitch(kVersionSwitchName)) {
    printf("%s\n", STRINGIZE(VERSION));
    return 0;
  }

  FilePath debug_log = remoting::GetConfigDir().
      Append(FILE_PATH_LITERAL("debug.log"));
  InitLogging(debug_log.value().c_str(),
#if defined(OS_WIN)
              logging::LOG_ONLY_TO_FILE,
#else
              logging::LOG_ONLY_TO_SYSTEM_DEBUG_LOG,
#endif
              logging::DONT_LOCK_LOG_FILE,
              logging::APPEND_TO_OLD_LOG_FILE,
              logging::DISABLE_DCHECK_FOR_NON_OFFICIAL_RELEASE_BUILDS);

#if defined(TOOLKIT_GTK)
  const CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  gfx::GtkInitFromCommandLine(*cmd_line);
#endif  // TOOLKIT_GTK

  net::EnableSSLServerSockets();

  MessageLoop message_loop(MessageLoop::TYPE_UI);
  base::Closure quit_message_loop = base::Bind(&QuitMessageLoop, &message_loop);
  scoped_ptr<remoting::ChromotingHostContext> context(
      new remoting::ChromotingHostContext(
          new remoting::AutoThreadTaskRunner(message_loop.message_loop_proxy(),
                                             quit_message_loop)));

#if defined(OS_LINUX)
  remoting::VideoFrameCapturer::EnableXDamage(true);
  remoting::AudioCapturerLinux::SetPipeName(CommandLine::ForCurrentProcess()->
      GetSwitchValuePath(kAudioPipeSwitchName));
#endif  // defined(OS_LINUX)

  if (!context->Start())
    return remoting::kHostInitializationFailed;

  remoting::HostProcess me2me_host(context.Pass());
  me2me_host.StartHostProcess();
  message_loop.Run();
  return me2me_host.get_exit_code();
}

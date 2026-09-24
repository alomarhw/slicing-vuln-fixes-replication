void PPAPITestBase::SetUpCommandLine(CommandLine* command_line) {
  command_line->AppendSwitch(switches::kEnableFileCookies);

  command_line->AppendSwitch(switches::kEnablePepperTesting);

  command_line->AppendSwitch(switches::kDisableSmoothScrolling);
}

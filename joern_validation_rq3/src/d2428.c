static void ZygotePreSandboxInit() {
  base::RandUint64();

  base::SysInfo::AmountOfPhysicalMemory();
  base::SysInfo::NumberOfProcessors();

  std::unique_ptr<icu::TimeZone> zone(icu::TimeZone::createDefault());

#if defined(ARCH_CPU_ARM_FAMILY)
  CRYPTO_library_init();
#endif

  RAND_set_urandom_fd(base::GetUrandomFD());

#if BUILDFLAG(ENABLE_PLUGINS)
  PreloadPepperPlugins();
#endif
#if BUILDFLAG(ENABLE_WEBRTC)
  InitializeWebRtcModule();
#endif

#if BUILDFLAG(ENABLE_CDM_HOST_VERIFICATION)
  CdmHostFiles::CreateGlobalInstance();
#endif

  SkFontConfigInterface::SetGlobal(
      new FontConfigIPC(GetSandboxFD()))->unref();

  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kAndroidFontsPath)) {
    std::string android_fonts_dir =
        base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
            switches::kAndroidFontsPath);

    if (android_fonts_dir.size() > 0 && android_fonts_dir.back() != '/')
      android_fonts_dir += '/';

    SkFontMgr_Android_CustomFonts custom;
    custom.fSystemFontUse =
        SkFontMgr_Android_CustomFonts::SystemFontUse::kOnlyCustom;
    custom.fBasePath = android_fonts_dir.c_str();

    std::string font_config;
    std::string fallback_font_config;
    if (android_fonts_dir.find("kitkat") != std::string::npos) {
      font_config = android_fonts_dir + "system_fonts.xml";
      fallback_font_config = android_fonts_dir + "fallback_fonts.xml";
      custom.fFallbackFontsXml = fallback_font_config.c_str();
    } else {
      font_config = android_fonts_dir + "fonts.xml";
      custom.fFallbackFontsXml = nullptr;
    }
    custom.fFontsXml = font_config.c_str();
    custom.fIsolated = true;

    blink::WebFontRendering::SetSkiaFontManager(SkFontMgr_New_Android(&custom));
  }
}

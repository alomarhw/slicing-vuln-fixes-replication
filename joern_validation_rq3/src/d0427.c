void ChromeContentBrowserClient::RegisterNonNetworkNavigationURLLoaderFactories(
    int frame_tree_node_id,
    NonNetworkURLLoaderFactoryMap* factories) {
#if BUILDFLAG(ENABLE_EXTENSIONS) || defined(OS_CHROMEOS)
  content::WebContents* web_contents =
      content::WebContents::FromFrameTreeNodeId(frame_tree_node_id);
#if BUILDFLAG(ENABLE_EXTENSIONS)
  factories->emplace(
      extensions::kExtensionScheme,
      extensions::CreateExtensionNavigationURLLoaderFactory(
          web_contents->GetBrowserContext(),
          !!extensions::WebViewGuest::FromWebContents(web_contents)));
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)
#if defined(OS_CHROMEOS)
  Profile* profile =
      Profile::FromBrowserContext(web_contents->GetBrowserContext());
  factories->emplace(
      content::kExternalFileScheme,
      std::make_unique<chromeos::ExternalFileURLLoaderFactory>(profile));
#endif  // defined(OS_CHROMEOS)
#endif  // BUILDFLAG(ENABLE_EXTENSIONS) || defined(OS_CHROMEOS)
}

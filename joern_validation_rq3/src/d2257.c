GURL ChromeContentBrowserClientExtensionsPart::GetEffectiveURL(
    Profile* profile, const GURL& url) {
  ExtensionRegistry* registry = ExtensionRegistry::Get(profile);
  if (!registry)
    return url;

  const Extension* extension =
      registry->enabled_extensions().GetHostedAppByURL(url);
  if (!extension)
    return url;

  if (extension->from_bookmark())
    return url;

  return extension->GetResourceURL(url.path());
}

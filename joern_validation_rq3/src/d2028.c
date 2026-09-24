ChromeExtensionsAPIClient::CreateDevicePermissionsPrompt(
    content::WebContents* web_contents) const {
  return base::MakeUnique<ChromeDevicePermissionsPrompt>(web_contents);
}

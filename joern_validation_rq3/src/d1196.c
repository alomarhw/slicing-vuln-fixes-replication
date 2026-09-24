ChromeContentBrowserClient::GetServiceManifestOverlay(base::StringPiece name) {
  ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
  int id = -1;
  if (name == content::mojom::kBrowserServiceName)
    id = IDR_CHROME_CONTENT_BROWSER_MANIFEST_OVERLAY;
  else if (name == content::mojom::kGpuServiceName)
    id = IDR_CHROME_CONTENT_GPU_MANIFEST_OVERLAY;
  else if (name == content::mojom::kPackagedServicesServiceName)
    id = IDR_CHROME_CONTENT_PACKAGED_SERVICES_MANIFEST_OVERLAY;
  else if (name == content::mojom::kPluginServiceName)
    id = IDR_CHROME_CONTENT_PLUGIN_MANIFEST_OVERLAY;
  else if (name == content::mojom::kRendererServiceName)
    id = IDR_CHROME_CONTENT_RENDERER_MANIFEST_OVERLAY;
  else if (name == content::mojom::kUtilityServiceName)
    id = IDR_CHROME_CONTENT_UTILITY_MANIFEST_OVERLAY;
  if (id == -1)
    return base::nullopt;

  base::StringPiece manifest_contents =
      rb.GetRawDataResourceForScale(id, ui::ScaleFactor::SCALE_FACTOR_NONE);
  return service_manager::Manifest::FromValueDeprecated(
      base::JSONReader::Read(manifest_contents));
}

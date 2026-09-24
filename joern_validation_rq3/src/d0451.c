std::string ChromeMetricsServiceClient::GetAppPackageName() {
#if defined(OS_ANDROID)
  return base::android::BuildInfo::GetInstance()->package_name();
#endif
  return std::string();
}

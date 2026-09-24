  PlatformSensorFusionTest() {
    provider_ = std::make_unique<FakePlatformSensorProvider>();
    PlatformSensorProvider::SetProviderForTesting(provider_.get());
  }

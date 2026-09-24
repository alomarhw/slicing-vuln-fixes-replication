  TestingDomainReliabilityServiceFactoryUserData(
      content::BrowserContext* context,
      MockDomainReliabilityService* service)
      : context(context),
        service(service),
        attached(false) {}

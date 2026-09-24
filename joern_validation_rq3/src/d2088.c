CaptureGroupNameHttpProxySocketPool::CaptureGroupNameSocketPool(
    HostResolver* host_resolver,
    CertVerifier* /* cert_verifier */)
    : HttpProxyClientSocketPool(
          0, 0, NULL, host_resolver, NULL, NULL, NULL, NULL) {}

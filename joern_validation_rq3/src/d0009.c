void ShellContentBrowserClient::SelectClientCertificate(
    WebContents* web_contents,
    net::SSLCertRequestInfo* cert_request_info,
    scoped_ptr<ClientCertificateDelegate> delegate) {
  if (!select_client_certificate_callback_.is_null())
    select_client_certificate_callback_.Run();
}

  void HTTPUploadDataOperationTest(const std::string& method) {
    const int kMsgSize = 20000;  // multiple of 10
    const int kIterations = 50;
    char* uploadBytes = new char[kMsgSize+1];
    char* ptr = uploadBytes;
    char marker = 'a';
    for (int idx = 0; idx < kMsgSize/10; idx++) {
      memcpy(ptr, "----------", 10);
      ptr += 10;
      if (idx % 100 == 0) {
        ptr--;
        *ptr++ = marker;
        if (++marker > 'z')
          marker = 'a';
      }
    }
    uploadBytes[kMsgSize] = '\0';

    for (int i = 0; i < kIterations; ++i) {
      TestDelegate d;
      URLRequest r(test_server_.GetURL("echo"), &d, &default_context_);
      r.set_method(method.c_str());

      r.AppendBytesToUpload(uploadBytes, kMsgSize);

      r.Start();
      EXPECT_TRUE(r.is_pending());

      MessageLoop::current()->Run();

      ASSERT_EQ(1, d.response_started_count()) << "request failed: " <<
          (int) r.status().status() << ", os error: " << r.status().error();

      EXPECT_FALSE(d.received_data_before_response());
      EXPECT_EQ(uploadBytes, d.data_received());
      EXPECT_EQ(memcmp(uploadBytes, d.data_received().c_str(), kMsgSize), 0);
      EXPECT_EQ(d.data_received().compare(uploadBytes), 0);
    }
    delete[] uploadBytes;
  }

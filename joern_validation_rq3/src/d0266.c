void DiskCacheBackendTest::BackendInvalidEntry8() {
  const int kSize = 0x3000;  // 12 kB
  SetMaxSize(kSize * 10);
  InitCache();

  std::string first("some key");
  std::string second("something else");
  disk_cache::Entry* entry;
  ASSERT_THAT(CreateEntry(first, &entry), IsOk());
  entry->Close();
  ASSERT_THAT(CreateEntry(second, &entry), IsOk());

  disk_cache::EntryImpl* entry_impl =
      static_cast<disk_cache::EntryImpl*>(entry);

  entry_impl->rankings()->Data()->contents = 0;
  entry_impl->rankings()->Store();
  entry->Close();
  FlushQueueForTest();
  EXPECT_EQ(2, cache_->GetEntryCount());

  EXPECT_NE(net::OK, OpenEntry(second, &entry));
  EXPECT_EQ(1, cache_->GetEntryCount());

  std::unique_ptr<TestIterator> iter = CreateIterator();
  ASSERT_THAT(iter->OpenNextEntry(&entry), IsOk());
  entry->Close();
  EXPECT_NE(net::OK, iter->OpenNextEntry(&entry));
  EXPECT_EQ(1, cache_->GetEntryCount());
}

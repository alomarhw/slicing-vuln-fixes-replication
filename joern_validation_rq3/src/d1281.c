std::string PPAPINaClTest::BuildQuery(const std::string& base,
                                      const std::string& test_case) {
  return StringPrintf("%smode=nacl&testcase=%s", base.c_str(),
                      test_case.c_str());
}

  void VerifyPageCount(int count) {
#if defined(OS_CHROMEOS)
#else
    const IPC::Message* page_cnt_msg =
        render_thread_.sink().GetUniqueMessageMatching(
            PrintHostMsg_DidGetPrintedPagesCount::ID);
    ASSERT_TRUE(page_cnt_msg);
    PrintHostMsg_DidGetPrintedPagesCount::Param post_page_count_param;
    PrintHostMsg_DidGetPrintedPagesCount::Read(page_cnt_msg,
                                               &post_page_count_param);
    EXPECT_EQ(count, post_page_count_param.b);
#endif  // defined(OS_CHROMEOS)
  }

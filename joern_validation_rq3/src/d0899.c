  void NotifyReply(webkit_glue::WebIntentReplyType reply_type,
                   const string16& data) {
    notified_reply_type_ = reply_type;
    notified_data_ = data;
  }

static void requestActivityNotification(AMediaCodec *codec) {
 (new AMessage(kWhatRequestActivityNotifications, codec->mHandler))->post();
}

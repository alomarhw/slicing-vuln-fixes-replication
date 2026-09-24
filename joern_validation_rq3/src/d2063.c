void CrosMock::InitMockSpeechSynthesisLibrary() {
  InitMockLibraryLoader();
  if (mock_speech_synthesis_library_)
    return;
  mock_speech_synthesis_library_ =
      new StrictMock<MockSpeechSynthesisLibrary>();
  test_api()->SetSpeechSynthesisLibrary(mock_speech_synthesis_library_, true);
}

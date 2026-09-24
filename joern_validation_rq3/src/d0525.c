void LocalFrame::Init() {
  CoreInitializer::GetInstance().InitLocalFrame(*this);

  loader_.Init();
}

void RenderViewTest::SimulateUserInputChangeForElement(
    blink::WebInputElement* input,
    const std::string& new_value) {
  ASSERT_TRUE(base::IsStringASCII(new_value));
  while (!input->focused())
    input->document().frame()->view()->advanceFocus(false);

  size_t previous_length = input->value().length();
  for (size_t i = 0; i < previous_length; ++i)
    SimulateUserTypingASCIICharacter(ui::VKEY_BACK, false);

  EXPECT_TRUE(input->value().utf8().empty());
  for (size_t i = 0; i < new_value.size(); ++i)
    SimulateUserTypingASCIICharacter(new_value[i], false);

  EXPECT_EQ(new_value, input->value().utf8().substr(0, new_value.length()));

  base::MessageLoop::current()->RunUntilIdle();
}

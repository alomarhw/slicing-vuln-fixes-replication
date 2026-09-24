std::unique_ptr<JSONObject> ScrollPaintPropertyNode::ToJSON() const {
  auto json = JSONObject::Create();
  if (Parent())
    json->SetString("parent", String::Format("%p", Parent()));
  if (state_.container_rect != IntRect())
    json->SetString("containerRect", state_.container_rect.ToString());
  if (state_.contents_rect != IntRect())
    json->SetString("contentsRect", state_.contents_rect.ToString());
  if (state_.user_scrollable_horizontal || state_.user_scrollable_vertical) {
    json->SetString(
        "userScrollable",
        state_.user_scrollable_horizontal
            ? (state_.user_scrollable_vertical ? "both" : "horizontal")
            : "vertical");
  }
  if (state_.main_thread_scrolling_reasons) {
    json->SetString(
        "mainThreadReasons",
        MainThreadScrollingReason::AsText(state_.main_thread_scrolling_reasons)
            .c_str());
  }
  if (state_.compositor_element_id) {
    json->SetString("compositorElementId",
                    state_.compositor_element_id.ToString().c_str());
  }
  return json;
}

void V8TestObject::DoubleOrStringAttributeAttributeSetterCallback(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  RUNTIME_CALL_TIMER_SCOPE_DISABLED_BY_DEFAULT(info.GetIsolate(), "Blink_TestObject_doubleOrStringAttribute_Setter");

  v8::Local<v8::Value> v8_value = info[0];

  test_object_v8_internal::DoubleOrStringAttributeAttributeSetter(v8_value, info);
}

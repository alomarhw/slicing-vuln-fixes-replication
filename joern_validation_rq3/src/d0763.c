static void customGetterReadonlyObjectAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    V8TestObjectPython::customGetterReadonlyObjectAttributeAttributeGetterCustom(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void perContextEnabledRuntimeEnabledLongAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestObjectPython* imp = V8TestObjectPython::toNative(info.Holder());
    v8SetReturnValueInt(info, imp->perContextEnabledRuntimeEnabledLongAttribute());
}

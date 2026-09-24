static void nodeAttributeAttributeSetter(v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    TestObjectPython* imp = V8TestObjectPython::toNative(info.Holder());
    V8TRYCATCH_VOID(Node*, cppValue, V8Node::toNativeWithTypeCheck(info.GetIsolate(), jsValue));
    imp->setNodeAttribute(WTF::getPtr(cppValue));
}

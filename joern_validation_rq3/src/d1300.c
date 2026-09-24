static void dateSequenceAttrAttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    INC_STATS("DOM.TestObj.dateSequenceAttr._set");
    TestObj* imp = V8TestObj::toNative(info.Holder());
    Vector<Date> v = toNativeArray<Date>(value);
    imp->setDateSequenceAttr(v);
    return;
}

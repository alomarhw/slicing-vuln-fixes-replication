JSValue jsTestObjXMLObjAttr(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSTestObj* castedThis = jsCast<JSTestObj*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    TestObj* impl = static_cast<TestObj*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(impl->xmlObjAttr()));
    return result;
}

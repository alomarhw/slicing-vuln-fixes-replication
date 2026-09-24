JSValue jsTestObjUnsignedLongLongAttr(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSTestObj* castedThis = jsCast<JSTestObj*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    TestObj* impl = static_cast<TestObj*>(castedThis->impl());
    JSValue result = jsNumber(impl->unsignedLongLongAttr());
    return result;
}

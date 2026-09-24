JSValue jsTestObjWithScriptStateAttribute(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSTestObj* castedThis = jsCast<JSTestObj*>(asObject(slotBase));
    TestObj* impl = static_cast<TestObj*>(castedThis->impl());
    JSValue result = jsNumber(impl->withScriptStateAttribute(exec));
    return result;
}

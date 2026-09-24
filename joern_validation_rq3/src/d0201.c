JSValue JSTestObj::getConstructor(ExecState* exec, JSGlobalObject* globalObject)
{
    return getDOMConstructor<JSTestObjConstructor>(exec, jsCast<JSDOMGlobalObject*>(globalObject));
}

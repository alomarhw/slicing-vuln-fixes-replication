void JSTestCustomNamedGetter::destroy(JSC::JSCell* cell)
{
    JSTestCustomNamedGetter* thisObject = jsCast<JSTestCustomNamedGetter*>(cell);
    thisObject->JSTestCustomNamedGetter::~JSTestCustomNamedGetter();
}

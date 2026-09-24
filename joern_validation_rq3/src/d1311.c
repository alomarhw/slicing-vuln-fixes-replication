bool JSTestInterfacePrototype::getOwnPropertyDescriptor(JSObject* object, ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    JSTestInterfacePrototype* thisObject = jsCast<JSTestInterfacePrototype*>(object);
    return getStaticPropertyDescriptor<JSTestInterfacePrototype, JSObject>(exec, &JSTestInterfacePrototypeTable, thisObject, propertyName, descriptor);
}

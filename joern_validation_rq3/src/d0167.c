bool JSFloat64Array::getOwnPropertySlot(JSCell* cell, ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    JSFloat64Array* thisObject = jsCast<JSFloat64Array*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, &s_info);
    bool ok;
    unsigned index = propertyName.toUInt32(ok);
    if (ok && index < static_cast<Float64Array*>(thisObject->impl())->length()) {
        slot.setValue(thisObject->getByIndex(exec, index));
        return true;
    }
    return getStaticValueSlot<JSFloat64Array, Base>(exec, getJSFloat64ArrayTable(exec), thisObject, propertyName, slot);
}

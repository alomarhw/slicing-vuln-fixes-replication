static bool NPN_SetProperty(NPP, NPObject* npObject, NPIdentifier propertyName, const NPVariant* value)
{
    if (npObject->_class->setProperty)
        return npObject->_class->setProperty(npObject, propertyName, value);

     return false;
 }

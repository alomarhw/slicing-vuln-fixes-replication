NO_INLINE JsVar *jspGetBuiltinPrototype(JsVar *obj) {
  if (jsvIsArray(obj)) {
    JsVar *v = jspFindPrototypeFor("Array");
    if (v) return v;
  }
  if (jsvIsObject(obj) || jsvIsArray(obj)) {
    JsVar *v = jspFindPrototypeFor("Object");
    if (v==obj) { // don't return ourselves
      jsvUnLock(v);
      v = 0;
    }
    return v;
  }
  return 0;
}

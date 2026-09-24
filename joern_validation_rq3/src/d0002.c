bool jsvIsVariableDefined(JsVar *a) {
  return !jsvIsName(a) ||
         jsvIsNameWithValue(a) ||
         (jsvGetFirstChild(a)!=0);
}

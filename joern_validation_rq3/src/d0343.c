void js_delproperty(js_State *J, int idx, const char *name)
{
	jsR_delproperty(J, js_toobject(J, idx), name);
}

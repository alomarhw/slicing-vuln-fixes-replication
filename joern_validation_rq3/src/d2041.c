JSValueRef WebFrame::computedStyleIncludingVisitedInfo(JSObjectRef element)
{
    if (!m_coreFrame)
        return 0;

    JSDOMWindow* globalObject = m_coreFrame->script()->globalObject(mainThreadNormalWorld());
    ExecState* exec = globalObject->globalExec();

    if (!toJS(element)->inherits(&JSElement::s_info))
        return JSValueMakeUndefined(toRef(exec));

    RefPtr<CSSComputedStyleDeclaration> style = computedStyle(static_cast<JSElement*>(toJS(element))->impl(), true);

    JSLock lock(SilenceAssertionsOnly);
    return toRef(exec, toJS(exec, globalObject, style.get()));
}

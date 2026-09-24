ScriptPromise AudioContext::resumeContext(ScriptState* scriptState)
{
    ASSERT(isMainThread());
    AutoLocker locker(this);

    if (isOfflineContext()) {
        return ScriptPromise::rejectWithDOMException(
            scriptState,
            DOMException::create(
                InvalidStateError,
                "cannot resume an OfflineAudioContext"));
    }

    RefPtrWillBeRawPtr<ScriptPromiseResolver> resolver = ScriptPromiseResolver::create(scriptState);
    ScriptPromise promise = resolver->promise();

    if (m_destinationNode)
        startRendering();

    m_resumeResolvers.append(resolver);

    return promise;
}

    Serializer(Writer& writer, MessagePortArray* messagePorts, ArrayBufferArray* arrayBuffers, WebBlobInfoArray* blobInfo, BlobDataHandleMap& blobDataHandles, v8::TryCatch& tryCatch, ScriptState* scriptState)
        : m_scriptState(scriptState)
        , m_writer(writer)
        , m_tryCatch(tryCatch)
        , m_depth(0)
        , m_status(Success)
        , m_nextObjectReference(0)
        , m_blobInfo(blobInfo)
        , m_blobDataHandles(blobDataHandles)
    {
        ASSERT(!tryCatch.HasCaught());
        v8::Handle<v8::Object> creationContext = m_scriptState->context()->Global();
        if (messagePorts) {
            for (size_t i = 0; i < messagePorts->size(); i++)
                m_transferredMessagePorts.set(toV8Object(messagePorts->at(i).get(), creationContext, isolate()), i);
        }
        if (arrayBuffers) {
            for (size_t i = 0; i < arrayBuffers->size(); i++)  {
                v8::Handle<v8::Object> v8ArrayBuffer = toV8Object(arrayBuffers->at(i).get(), creationContext, isolate());
                if (!m_transferredArrayBuffers.contains(v8ArrayBuffer))
                    m_transferredArrayBuffers.set(v8ArrayBuffer, i);
            }
        }
    }

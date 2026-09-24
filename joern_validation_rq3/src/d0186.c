v8::Handle<v8::Value> SerializedScriptValue::deserialize(v8::Isolate* isolate, MessagePortArray* messagePorts, const WebBlobInfoArray* blobInfo)
{
    if (!m_data.impl())
        return v8::Null(isolate);
    COMPILE_ASSERT(sizeof(BufferValueType) == 2, BufferValueTypeIsTwoBytes);
    m_data.ensure16Bit();
    Reader reader(reinterpret_cast<const uint8_t*>(m_data.impl()->characters16()), 2 * m_data.length(), blobInfo, m_blobDataHandles, ScriptState::current(isolate));
    Deserializer deserializer(reader, messagePorts, m_arrayBufferContentsArray.get());

    RefPtr<SerializedScriptValue> protect(this);
    return deserializer.deserialize();
}

void ScriptProfiler::visitNodeWrappers(WrappedNodeVisitor* visitor)
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);

    class DOMNodeWrapperVisitor : public v8::PersistentHandleVisitor {
    public:
        DOMNodeWrapperVisitor(WrappedNodeVisitor* visitor, v8::Isolate* isolate)
            : m_visitor(visitor)
            , m_isolate(isolate)
        {
        }

        virtual void VisitPersistentHandle(v8::Persistent<v8::Value>* value, uint16_t classId) OVERRIDE
        {
            if (classId != v8DOMNodeClassId)
                return;
            v8::Handle<v8::Object>* wrapper = reinterpret_cast<v8::Handle<v8::Object>*>(value);
            ASSERT(V8Node::HasInstanceInAnyWorld(*wrapper, m_isolate));
            ASSERT((*wrapper)->IsObject());
            m_visitor->visitNode(V8Node::toNative(*wrapper));
        }

    private:
        WrappedNodeVisitor* m_visitor;
        v8::Isolate* m_isolate;
    } wrapperVisitor(visitor, isolate);

    v8::V8::VisitHandlesWithClassIds(&wrapperVisitor);
}

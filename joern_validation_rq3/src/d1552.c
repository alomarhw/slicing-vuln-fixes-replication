void Document::postInspectorTask(const WebTraceLocation& location, std::unique_ptr<ExecutionContextTask> task)
{
    m_taskRunner->postInspectorTask(location, std::move(task));
}

void Document::postTask(PassOwnPtr<ExecutionContextTask> task)
{
    callOnMainThread(didReceiveTask, new PerformTaskContext(m_weakFactory.createWeakPtr(), task));
}

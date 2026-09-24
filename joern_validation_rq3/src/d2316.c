    void appendEndElementNSCallback()
    {
        PendingEndElementNSCallback* callback = new PendingEndElementNSCallback;
        
        m_callbacks.append(callback);
    }

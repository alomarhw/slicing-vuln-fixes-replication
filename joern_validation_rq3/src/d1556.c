void WebGraphicsContext3DDefaultImpl::vertexAttribPointer(unsigned long indx, int size, int type, bool normalized,
                                                          unsigned long stride, unsigned long offset)
{
    makeContextCurrent();

    if (m_boundArrayBuffer <= 0) {
        return;
    }

    if (indx < NumTrackedPointerStates) {
        VertexAttribPointerState& state = m_vertexAttribPointerState[indx];
        state.buffer = m_boundArrayBuffer;
        state.indx = indx;
        state.size = size;
        state.type = type;
        state.normalized = normalized;
        state.stride = stride;
        state.offset = offset;
    }

    glVertexAttribPointer(indx, size, type, normalized, stride,
                          reinterpret_cast<void*>(static_cast<intptr_t>(offset)));
}

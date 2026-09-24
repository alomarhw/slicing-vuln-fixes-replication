void GpuCommandBufferStub::SendMemoryAllocationToProxy(
    const GpuMemoryAllocation& allocation) {
  Send(new GpuCommandBufferMsg_SetMemoryAllocation(route_id_, allocation));
}

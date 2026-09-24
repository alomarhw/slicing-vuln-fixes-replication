viz::LocalSurfaceIdAllocation CompositorImpl::GenerateLocalSurfaceId() const {
  if (enable_surface_synchronization_) {
    viz::ParentLocalSurfaceIdAllocator& allocator =
        CompositorDependencies::Get().surface_id_allocator;
    allocator.GenerateId();
    return allocator.GetCurrentLocalSurfaceIdAllocation();
  }

  return viz::LocalSurfaceIdAllocation();
}

  AXTreeSnapshotCallback AddFrame(bool is_root) {
    return base::BindOnce(&AXTreeSnapshotCombiner::ReceiveSnapshot, this,
                          is_root);
  }

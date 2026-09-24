pp::Var Instance::GetInstanceObject() {
  if (instance_object_.is_undefined()) {
    PDFScriptableObject* object = new PDFScriptableObject(this);
    instance_object_ = pp::VarPrivate(this, object);
  }

  return instance_object_;
}

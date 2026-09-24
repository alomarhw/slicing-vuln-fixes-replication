  void CreateDeviceAssociation(const std::string& input_device_id,
                               const std::string& output_device_id) {
    EXPECT_FALSE(IsValidDeviceId(input_device_id));
    EXPECT_FALSE(IsValidDeviceId(output_device_id));

    associations_[input_device_id] = output_device_id;
  }

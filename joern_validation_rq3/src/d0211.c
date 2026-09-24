 DeviceOrientationData* InspectorPageAgent::overrideDeviceOrientation(DeviceOrientationData* deviceOrientation)
{
    if (m_deviceOrientation)
        deviceOrientation = m_deviceOrientation.get();
    return deviceOrientation;
}

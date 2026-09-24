void BluetoothAdapter::NotifyGattCharacteristicRemoved(
    BluetoothRemoteGattCharacteristic* characteristic) {
  DCHECK_EQ(characteristic->GetService()->GetDevice()->GetAdapter(), this);

  for (auto& observer : observers_)
    observer.GattCharacteristicRemoved(this, characteristic);
}

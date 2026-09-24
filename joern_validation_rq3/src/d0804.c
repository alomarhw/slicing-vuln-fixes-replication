NTSTATUS TCDispatchQueueIRP (PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	PEXTENSION Extension = (PEXTENSION) DeviceObject->DeviceExtension;
	PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation (Irp);
	NTSTATUS ntStatus;

#if defined(_DEBUG) || defined (_DEBUG_TRACE)
	if (irpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL && (Extension->bRootDevice || Extension->IsVolumeDevice))
	{
		switch (irpSp->Parameters.DeviceIoControl.IoControlCode)
		{
		case TC_IOCTL_GET_MOUNTED_VOLUMES:
		case TC_IOCTL_GET_PASSWORD_CACHE_STATUS:
		case TC_IOCTL_GET_PORTABLE_MODE_STATUS:
		case TC_IOCTL_SET_PORTABLE_MODE_STATUS:
		case TC_IOCTL_OPEN_TEST:
		case TC_IOCTL_GET_RESOLVED_SYMLINK:
		case TC_IOCTL_GET_DEVICE_REFCOUNT:
		case TC_IOCTL_GET_DRIVE_PARTITION_INFO:
		case TC_IOCTL_GET_BOOT_DRIVE_VOLUME_PROPERTIES:
		case TC_IOCTL_GET_BOOT_ENCRYPTION_STATUS:
		case TC_IOCTL_GET_DECOY_SYSTEM_WIPE_STATUS:
		case TC_IOCTL_GET_WARNING_FLAGS:
		case TC_IOCTL_IS_HIDDEN_SYSTEM_RUNNING:
		case IOCTL_DISK_CHECK_VERIFY:
			break;

		default:
			Dump ("%ls (0x%x %d)\n",
				TCTranslateCode (irpSp->Parameters.DeviceIoControl.IoControlCode),
				(int) (irpSp->Parameters.DeviceIoControl.IoControlCode >> 16),
				(int) ((irpSp->Parameters.DeviceIoControl.IoControlCode & 0x1FFF) >> 2));
		}
	}
#endif

	if (!Extension->bRootDevice)
	{
		if (Extension->IsDriveFilterDevice)
			return DriveFilterDispatchIrp (DeviceObject, Irp);

		if (Extension->IsVolumeFilterDevice)
			return VolumeFilterDispatchIrp (DeviceObject, Irp);
	}

	switch (irpSp->MajorFunction)
	{
	case IRP_MJ_CLOSE:
	case IRP_MJ_CREATE:
	case IRP_MJ_CLEANUP:
		return COMPLETE_IRP (DeviceObject, Irp, STATUS_SUCCESS, 0);

	case IRP_MJ_SHUTDOWN:
		if (Extension->bRootDevice)
		{
			Dump ("Driver shutting down\n");
			DriverShuttingDown = TRUE;

			if (EncryptionSetupThread)
				while (SendDeviceIoControlRequest (RootDeviceObject, TC_IOCTL_ABORT_BOOT_ENCRYPTION_SETUP, NULL, 0, NULL, 0) == STATUS_INSUFFICIENT_RESOURCES);

			if (DecoySystemWipeThread)
				while (SendDeviceIoControlRequest (RootDeviceObject, TC_IOCTL_ABORT_DECOY_SYSTEM_WIPE, NULL, 0, NULL, 0) == STATUS_INSUFFICIENT_RESOURCES);

			OnShutdownPending();
		}

		return COMPLETE_IRP (DeviceObject, Irp, STATUS_SUCCESS, 0);

	case IRP_MJ_FLUSH_BUFFERS:
	case IRP_MJ_READ:
	case IRP_MJ_WRITE:
	case IRP_MJ_DEVICE_CONTROL:

		if (Extension->bRootDevice)
		{
			if (irpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL)
			{
				NTSTATUS status = KeWaitForMutexObject (&RootDeviceControlMutex, Executive, KernelMode, FALSE, NULL);
				if (!NT_SUCCESS (status))
					return status;

				status = ProcessMainDeviceControlIrp (DeviceObject, Extension, Irp);

				KeReleaseMutex (&RootDeviceControlMutex, FALSE);
				return status;
			}
			break;
		}

		if (Extension->bShuttingDown)
		{
			Dump ("Device %d shutting down: STATUS_DELETE_PENDING\n", Extension->nDosDriveNo);
			return TCCompleteDiskIrp (Irp, STATUS_DELETE_PENDING, 0);
		}

		if (Extension->bRemovable
			&& (DeviceObject->Flags & DO_VERIFY_VOLUME)
			&& !(irpSp->Flags & SL_OVERRIDE_VERIFY_VOLUME)
			&& irpSp->MajorFunction != IRP_MJ_FLUSH_BUFFERS)
		{
			Dump ("Removable device %d has DO_VERIFY_VOLUME flag: STATUS_DEVICE_NOT_READY\n", Extension->nDosDriveNo);
			return TCCompleteDiskIrp (Irp, STATUS_DEVICE_NOT_READY, 0);
		}

		switch (irpSp->MajorFunction)
		{
		case IRP_MJ_READ:
		case IRP_MJ_WRITE:
			ntStatus = EncryptedIoQueueAddIrp (&Extension->Queue, Irp);

			if (ntStatus != STATUS_PENDING)
				TCCompleteDiskIrp (Irp, ntStatus, 0);

			return ntStatus;

		case IRP_MJ_DEVICE_CONTROL:
			ntStatus = IoAcquireRemoveLock (&Extension->Queue.RemoveLock, Irp);
			if (!NT_SUCCESS (ntStatus))
				return TCCompleteIrp (Irp, ntStatus, 0);

			IoMarkIrpPending (Irp);

			ExInterlockedInsertTailList (&Extension->ListEntry, &Irp->Tail.Overlay.ListEntry, &Extension->ListSpinLock);
			KeReleaseSemaphore (&Extension->RequestSemaphore, IO_DISK_INCREMENT, 1, FALSE);

			return STATUS_PENDING;

		case IRP_MJ_FLUSH_BUFFERS:
			return TCCompleteDiskIrp (Irp, STATUS_SUCCESS, 0);
		}

		break;

	case IRP_MJ_PNP:
		if (!Extension->bRootDevice
			&& Extension->IsVolumeDevice
			&& irpSp->MinorFunction == IRP_MN_DEVICE_USAGE_NOTIFICATION
			&& irpSp->Parameters.UsageNotification.Type == DeviceUsageTypePaging
			&& irpSp->Parameters.UsageNotification.InPath)
		{
			PagingFileCreationPrevented = TRUE;
			return TCCompleteIrp (Irp, STATUS_UNSUCCESSFUL, 0);
		}
		break;
	}

	return TCCompleteIrp (Irp, STATUS_INVALID_DEVICE_REQUEST, 0);
}

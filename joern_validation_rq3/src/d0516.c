ssize_t tpm_show_caps(struct device *dev, struct device_attribute *attr,
		      char *buf)
{
	cap_t cap;
	ssize_t rc;
	char *str = buf;

	rc = tpm_getcap(dev, TPM_CAP_PROP_MANUFACTURER, &cap,
			"attempting to determine the manufacturer");
	if (rc)
		return 0;
	str += sprintf(str, "Manufacturer: 0x%x\n",
		       be32_to_cpu(cap.manufacturer_id));

	rc = tpm_getcap(dev, CAP_VERSION_1_1, &cap,
		        "attempting to determine the 1.1 version");
	if (rc)
		return 0;
	str += sprintf(str,
		       "TCG version: %d.%d\nFirmware version: %d.%d\n",
		       cap.tpm_version.Major, cap.tpm_version.Minor,
		       cap.tpm_version.revMajor, cap.tpm_version.revMinor);
	return str - buf;
}

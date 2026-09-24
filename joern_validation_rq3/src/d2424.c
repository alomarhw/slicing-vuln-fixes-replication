gsf_infile_tar_set_property (GObject      *object,
			     guint         property_id,
			     GValue const *value,
			     GParamSpec   *pspec)
{
	GsfInfileTar *tar = (GsfInfileTar *)object;

	switch (property_id) {
	case PROP_SOURCE:
		gsf_infile_tar_set_source (tar, g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

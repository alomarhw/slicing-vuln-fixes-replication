int rdma_resolve_route(struct rdma_cm_id *id, int timeout_ms)
{
	struct rdma_id_private *id_priv;
	int ret;

	id_priv = container_of(id, struct rdma_id_private, id);
	if (!cma_comp_exch(id_priv, RDMA_CM_ADDR_RESOLVED, RDMA_CM_ROUTE_QUERY))
		return -EINVAL;

	atomic_inc(&id_priv->refcount);
	switch (rdma_node_get_transport(id->device->node_type)) {
	case RDMA_TRANSPORT_IB:
		switch (rdma_port_get_link_layer(id->device, id->port_num)) {
		case IB_LINK_LAYER_INFINIBAND:
			ret = cma_resolve_ib_route(id_priv, timeout_ms);
			break;
		case IB_LINK_LAYER_ETHERNET:
			ret = cma_resolve_iboe_route(id_priv);
			break;
		default:
			ret = -ENOSYS;
		}
		break;
	case RDMA_TRANSPORT_IWARP:
		ret = cma_resolve_iw_route(id_priv, timeout_ms);
		break;
	default:
		ret = -ENOSYS;
		break;
	}
	if (ret)
		goto err;

	return 0;
err:
	cma_comp_exch(id_priv, RDMA_CM_ROUTE_QUERY, RDMA_CM_ADDR_RESOLVED);
	cma_deref_id(id_priv);
	return ret;
}

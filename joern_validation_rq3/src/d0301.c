static struct svc_xprt *svc_rdma_bc_create(struct svc_serv *serv,
					   struct net *net,
					   struct sockaddr *sa, int salen,
					   int flags)
{
	struct svcxprt_rdma *cma_xprt;
	struct svc_xprt *xprt;

	cma_xprt = rdma_create_xprt(serv, 0);
	if (!cma_xprt)
		return ERR_PTR(-ENOMEM);
	xprt = &cma_xprt->sc_xprt;

	svc_xprt_init(net, &svc_rdma_bc_class, xprt, serv);
	set_bit(XPT_CONG_CTRL, &xprt->xpt_flags);
	serv->sv_bc_xprt = xprt;

	dprintk("svcrdma: %s(%p)\n", __func__, xprt);
	return xprt;
}

jas_image_t *jas_image_chclrspc(jas_image_t *image, jas_cmprof_t *outprof,
  int intent)
{
	jas_image_t *inimage;
	int minhstep;
	int minvstep;
	int i;
	int j;
	int k;
	int n;
	int hstep;
	int vstep;
	int numinauxchans;
	int numoutauxchans;
	int numinclrchans;
	int numoutclrchans;
	int prec;
	jas_image_t *outimage;
	int cmpttype;
	int numoutchans;
	jas_cmprof_t *inprof;
	jas_cmprof_t *tmpprof;
	jas_image_cmptparm_t cmptparm;
	int width;
	int height;
	jas_cmxform_t *xform;
	jas_cmpixmap_t inpixmap;
	jas_cmpixmap_t outpixmap;
	jas_cmcmptfmt_t *incmptfmts;
	jas_cmcmptfmt_t *outcmptfmts;

#if 0
jas_eprintf("IMAGE\n");
jas_image_dump(image, stderr);
#endif

	outimage = 0;
	xform = 0;
	if (!(inimage = jas_image_copy(image)))
		goto error;
	image = 0;

	if (!jas_image_ishomosamp(inimage)) {
		minhstep = jas_image_cmpthstep(inimage, 0);
		minvstep = jas_image_cmptvstep(inimage, 0);
		for (i = 1; i < jas_image_numcmpts(inimage); ++i) {
			hstep = jas_image_cmpthstep(inimage, i);
			vstep = jas_image_cmptvstep(inimage, i);
			if (hstep < minhstep) {
				minhstep = hstep;
			}
			if (vstep < minvstep) {
				minvstep = vstep;
			}
		}
		n = jas_image_numcmpts(inimage);
		for (i = 0; i < n; ++i) {
			cmpttype = jas_image_cmpttype(inimage, i);
			if (jas_image_sampcmpt(inimage, i, i + 1, 0, 0, minhstep, minvstep,
			  jas_image_cmptsgnd(inimage, i), jas_image_cmptprec(inimage, i))) {
				goto error;
			}
			jas_image_setcmpttype(inimage, i + 1, cmpttype);
			jas_image_delcmpt(inimage, i);
		}
	}

	width = jas_image_cmptwidth(inimage, 0);
	height = jas_image_cmptheight(inimage, 0);
	hstep = jas_image_cmpthstep(inimage, 0);
	vstep = jas_image_cmptvstep(inimage, 0);

	if (!(inprof = jas_image_cmprof(inimage))) {
		abort();
	}
	numinclrchans = jas_clrspc_numchans(jas_cmprof_clrspc(inprof));
	numinauxchans = jas_image_numcmpts(inimage) - numinclrchans;
	numoutclrchans = jas_clrspc_numchans(jas_cmprof_clrspc(outprof));
	numoutauxchans = 0;
	numoutchans = numoutclrchans + numoutauxchans;
	prec = 8;

	if (!(outimage = jas_image_create0())) {
		goto error;
	}

	/* Create a component for each of the colorants. */
	for (i = 0; i < numoutclrchans; ++i) {
		cmptparm.tlx = 0;
		cmptparm.tly = 0;
		cmptparm.hstep = hstep;
		cmptparm.vstep = vstep;
		cmptparm.width = width;
		cmptparm.height = height;
		cmptparm.prec = prec;
		cmptparm.sgnd = 0;
		if (jas_image_addcmpt(outimage, -1, &cmptparm))
			goto error;
		jas_image_setcmpttype(outimage, i, JAS_IMAGE_CT_COLOR(i));
	}
#if 0
	/* Copy the auxiliary components without modification. */
	for (i = 0; i < jas_image_numcmpts(inimage); ++i) {
		if (!ISCOLOR(jas_image_cmpttype(inimage, i))) {
			jas_image_copycmpt(outimage, -1, inimage, i);
/* XXX - need to specify laydown of component on ref. grid */
		}
	}
#endif

	if (!(tmpprof = jas_cmprof_copy(outprof)))
		goto error;
	assert(!jas_image_cmprof(outimage));
	jas_image_setcmprof(outimage, tmpprof);
	tmpprof = 0;
	jas_image_setclrspc(outimage, jas_cmprof_clrspc(outprof));

	if (!(xform = jas_cmxform_create(inprof, outprof, 0, JAS_CMXFORM_OP_FWD,
	  intent, 0))) {
		goto error;
	}

	inpixmap.numcmpts = numinclrchans;
	if (!(incmptfmts = jas_alloc2(numinclrchans, sizeof(jas_cmcmptfmt_t)))) {
		abort();
	}
	inpixmap.cmptfmts = incmptfmts;
	for (i = 0; i < numinclrchans; ++i) {
		j = jas_image_getcmptbytype(inimage, JAS_IMAGE_CT_COLOR(i));
		assert(j >= 0);
		if (!(incmptfmts[i].buf = jas_alloc2(width, sizeof(long)))) {
			goto error;
		}
		incmptfmts[i].prec = jas_image_cmptprec(inimage, j);
		incmptfmts[i].sgnd = jas_image_cmptsgnd(inimage, j);
		incmptfmts[i].width = width;
		incmptfmts[i].height = 1;
	}

	outpixmap.numcmpts = numoutclrchans;
	if (!(outcmptfmts = jas_alloc2(numoutclrchans, sizeof(jas_cmcmptfmt_t)))) {
		abort();
	}
	outpixmap.cmptfmts = outcmptfmts;

	for (i = 0; i < numoutclrchans; ++i) {
		j = jas_image_getcmptbytype(outimage, JAS_IMAGE_CT_COLOR(i));
		assert(j >= 0);
		if (!(outcmptfmts[i].buf = jas_alloc2(width, sizeof(long))))
			goto error;
		outcmptfmts[i].prec = jas_image_cmptprec(outimage, j);
		outcmptfmts[i].sgnd = jas_image_cmptsgnd(outimage, j);
		outcmptfmts[i].width = width;
		outcmptfmts[i].height = 1;
	}

	for (i = 0; i < height; ++i) {
		for (j = 0; j < numinclrchans; ++j) {
			k = jas_image_getcmptbytype(inimage, JAS_IMAGE_CT_COLOR(j));
			if (jas_image_readcmpt2(inimage, k, 0, i, width, 1,
			  incmptfmts[j].buf))
				goto error;
		}
		jas_cmxform_apply(xform, &inpixmap, &outpixmap);
		for (j = 0; j < numoutclrchans; ++j) {
			k = jas_image_getcmptbytype(outimage, JAS_IMAGE_CT_COLOR(j));
			if (jas_image_writecmpt2(outimage, k, 0, i, width, 1,
			  outcmptfmts[j].buf))
				goto error;
		}
	}

	for (i = 0; i < numoutclrchans; ++i) {
		jas_free(outcmptfmts[i].buf);
	}
	jas_free(outcmptfmts);
	for (i = 0; i < numinclrchans; ++i) {
		jas_free(incmptfmts[i].buf);
	}
	jas_free(incmptfmts);
	jas_cmxform_destroy(xform);
	jas_image_destroy(inimage);

#if 0
jas_eprintf("INIMAGE\n");
jas_image_dump(inimage, stderr);
jas_eprintf("OUTIMAGE\n");
jas_image_dump(outimage, stderr);
#endif
	return outimage;
error:
	if (xform)
		jas_cmxform_destroy(xform);
	if (inimage)
		jas_image_destroy(inimage);
	if (outimage)
		jas_image_destroy(outimage);
	return 0;
}

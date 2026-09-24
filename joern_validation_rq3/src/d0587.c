gdImagePtr gdImageRotateBilinear(gdImagePtr src, const float degrees, const int bgColor)
{
	float _angle = (float)((- degrees / 180.0f) * M_PI);
	const unsigned int src_w = gdImageSX(src);
	const unsigned int src_h = gdImageSY(src);
	unsigned int new_width = abs((int)(src_w*cos(_angle))) + abs((int)(src_h*sin(_angle) + 0.5f));
	unsigned int new_height = abs((int)(src_w*sin(_angle))) + abs((int)(src_h*cos(_angle) + 0.5f));
	const gdFixed f_0_5 = gd_ftofx(0.5f);
	const gdFixed f_H = gd_itofx(src_h/2);
	const gdFixed f_W = gd_itofx(src_w/2);
	const gdFixed f_cos = gd_ftofx(cos(-_angle));
	const gdFixed f_sin = gd_ftofx(sin(-_angle));
	const gdFixed f_1 = gd_itofx(1);
	unsigned int i;
	unsigned int dst_offset_x;
	unsigned int dst_offset_y = 0;
	unsigned int src_offset_x, src_offset_y;
	gdImagePtr dst;

	dst = gdImageCreateTrueColor(new_width, new_height);
	if (dst == NULL) {
		return NULL;
	}
	dst->saveAlphaFlag = 1;

	for (i = 0; i < new_height; i++) {
		unsigned int j;
		dst_offset_x = 0;

		for (j=0; j < new_width; j++) {
			const gdFixed f_i = gd_itofx((int)i - (int)new_height/2);
			const gdFixed f_j = gd_itofx((int)j - (int)new_width/2);
			const gdFixed f_m = gd_mulfx(f_j,f_sin) + gd_mulfx(f_i,f_cos) + f_0_5 + f_H;
			const gdFixed f_n = gd_mulfx(f_j,f_cos) - gd_mulfx(f_i,f_sin) + f_0_5 + f_W;
			const unsigned int m = gd_fxtoi(f_m);
			const unsigned int n = gd_fxtoi(f_n);

			if ((m > 0) && (m < src_h - 1) && (n > 0) && (n < src_w - 1)) {
				const gdFixed f_f = f_m - gd_itofx(m);
				const gdFixed f_g = f_n - gd_itofx(n);
				const gdFixed f_w1 = gd_mulfx(f_1-f_f, f_1-f_g);
				const gdFixed f_w2 = gd_mulfx(f_1-f_f, f_g);
				const gdFixed f_w3 = gd_mulfx(f_f, f_1-f_g);
				const gdFixed f_w4 = gd_mulfx(f_f, f_g);

				if (n < src_w - 1) {
					src_offset_x = n + 1;
					src_offset_y = m;
				}

				if (m < src_h-1) {
					src_offset_x = n;
					src_offset_y = m + 1;
				}

				if (!((n >= src_w-1) || (m >= src_h-1))) {
					src_offset_x = n + 1;
					src_offset_y = m + 1;
				}
				{
					const int pixel1 = src->tpixels[src_offset_y][src_offset_x];
					register int pixel2, pixel3, pixel4;

					if (src_offset_y + 1 >= src_h) {
						pixel2 = bgColor;
						pixel3 = bgColor;
						pixel4 = bgColor;
					} else if (src_offset_x + 1 >= src_w) {
						pixel2 = bgColor;
						pixel3 = bgColor;
						pixel4 = bgColor;
					} else {
					    pixel2 = src->tpixels[src_offset_y][src_offset_x + 1];
						pixel3 = src->tpixels[src_offset_y + 1][src_offset_x];
						pixel4 = src->tpixels[src_offset_y + 1][src_offset_x + 1];
					}
					{
						const gdFixed f_r1 = gd_itofx(gdTrueColorGetRed(pixel1));
						const gdFixed f_r2 = gd_itofx(gdTrueColorGetRed(pixel2));
						const gdFixed f_r3 = gd_itofx(gdTrueColorGetRed(pixel3));
						const gdFixed f_r4 = gd_itofx(gdTrueColorGetRed(pixel4));
						const gdFixed f_g1 = gd_itofx(gdTrueColorGetGreen(pixel1));
						const gdFixed f_g2 = gd_itofx(gdTrueColorGetGreen(pixel2));
						const gdFixed f_g3 = gd_itofx(gdTrueColorGetGreen(pixel3));
						const gdFixed f_g4 = gd_itofx(gdTrueColorGetGreen(pixel4));
						const gdFixed f_b1 = gd_itofx(gdTrueColorGetBlue(pixel1));
						const gdFixed f_b2 = gd_itofx(gdTrueColorGetBlue(pixel2));
						const gdFixed f_b3 = gd_itofx(gdTrueColorGetBlue(pixel3));
						const gdFixed f_b4 = gd_itofx(gdTrueColorGetBlue(pixel4));
						const gdFixed f_a1 = gd_itofx(gdTrueColorGetAlpha(pixel1));
						const gdFixed f_a2 = gd_itofx(gdTrueColorGetAlpha(pixel2));
						const gdFixed f_a3 = gd_itofx(gdTrueColorGetAlpha(pixel3));
						const gdFixed f_a4 = gd_itofx(gdTrueColorGetAlpha(pixel4));
						const gdFixed f_red = gd_mulfx(f_w1, f_r1) + gd_mulfx(f_w2, f_r2) + gd_mulfx(f_w3, f_r3) + gd_mulfx(f_w4, f_r4);
						const gdFixed f_green = gd_mulfx(f_w1, f_g1) + gd_mulfx(f_w2, f_g2) + gd_mulfx(f_w3, f_g3) + gd_mulfx(f_w4, f_g4);
						const gdFixed f_blue = gd_mulfx(f_w1, f_b1) + gd_mulfx(f_w2, f_b2) + gd_mulfx(f_w3, f_b3) + gd_mulfx(f_w4, f_b4);
						const gdFixed f_alpha = gd_mulfx(f_w1, f_a1) + gd_mulfx(f_w2, f_a2) + gd_mulfx(f_w3, f_a3) + gd_mulfx(f_w4, f_a4);

						const unsigned char red   = (unsigned char) CLAMP(gd_fxtoi(f_red),   0, 255);
						const unsigned char green = (unsigned char) CLAMP(gd_fxtoi(f_green), 0, 255);
						const unsigned char blue  = (unsigned char) CLAMP(gd_fxtoi(f_blue),  0, 255);
						const unsigned char alpha = (unsigned char) CLAMP(gd_fxtoi(f_alpha), 0, 127);

						dst->tpixels[dst_offset_y][dst_offset_x++] = gdTrueColorAlpha(red, green, blue, alpha);
					}
				}
			} else {
				dst->tpixels[dst_offset_y][dst_offset_x++] = bgColor;
			}
		}
		dst_offset_y++;
	}
	return dst;
}

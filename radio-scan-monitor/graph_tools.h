
#define uint8 unsigned char

int goodRound(double x)
{
	double ax = x;
	if(ax < 0) ax = -ax;
	int i = ax;
	if(ax-i > 0.5) i++;
	if(x < 0) return -i;
	return i;
}

void getLNpoints(int W, int H, int x0, int y0, int x1, int y1, int *pz, int *N)
{
	int xy = abs(x1-x0) < abs(y1-y0), t;
	int pcount = 0;
	int step = 1;
	if(xy)
	{
		t = y0; y0 = x0; x0 = t;
		t = y1; y1 = x1; x1 = t;
	}
	if(x0 > x1)
	{
		step = -1;
//		t = x0; x0 = x1; x1 = t;
//		t = y0; y0 = y1; y1 = t;
	}
	int dx = abs(x1-x0);
	int dy = abs(y1-y0);
	int err = dx/2;
	int ys, cy = y0;
	int ptx, pty;
	ys = -1;
	if(y0 < y1) ys = 1;
//	for(int cx = x0; cx <= x1; ++ cx)
	for(int cx = x0; cx != x1; cx += step)
	{
		if(!xy)
		{
			ptx = cx;
			pty = cy;
		}
		else
		{
			ptx = cy;
			pty = cx;
		}
		if(cy >= 0 && cy < H && cx >= 0 && cx < W)
		{
			pz[pcount] = pty*W + ptx;
			pcount++;
		}
		err -= dy;
		if(err < 0)
		{
			cy += ys;
			err += dx;
		}
	}
	*N = pcount;
}

void grp_drawLN(uint8 *drawBuf, int W, int H, int x0, int y0, int x1, int y1, int R, int G, int B)
{
	int xy = abs(x1-x0) < abs(y1-y0), t;
	int step = 1;
	if(xy)
	{
		t = y0; y0 = x0; x0 = t;
		t = y1; y1 = x1; x1 = t;
	}
	if(x0 > x1)
	{
		step = -1;
//		t = x0; x0 = x1; x1 = t;
//		t = y0; y0 = y1; y1 = t;
	}
	int dx = abs(x1-x0);
	int dy = abs(y1-y0);
	int err = dx/2;
	int ys, cy = y0;
	ys = -1;
	if(y0 < y1) ys = 1;
//	for(int cx = x0; cx <= x1; ++ cx)
	for(int cx = x0; cx != x1; cx += step)
	{
		int idx = 0;
		if(!xy)
		{
			idx = cy*W + cx;
			idx *= 4;
			if(cy < 0 || cy >= H || cx < 0 || cx >= W) idx = -1;
		}
		else
		{
			idx = cx*W + cy;
			idx *= 4;
			if(cy < 0 || cy >= W || cx < 0 || cx >= H) idx = -1;
		}
		if(idx >= 0)
		{
			drawBuf[idx] = B;
			drawBuf[idx+1] = G;
			drawBuf[idx+2] = R;
			drawBuf[idx+3] = 0;			
		}
		err -= dy;
		if(err < 0)
		{
			cy += ys;
			err += dx;
		}
	}
}

inline uint8 byteClip(int v)
{
	v = (v<0)?0:v;
	v = (v>255)?255:v;
	return v;
}

void fastBlur4(uint8 *img, int w, int h, int size)
{
	int x,y;//,m;
        int m=size;
	int divZ = 0;
	int canFastDiv = 0;
	if(size == 1) divZ = 1, canFastDiv = 1;
	if(size == 3) divZ = 2, canFastDiv = 1;
	if(size == 7) divZ = 3, canFastDiv = 1;
	if(size == 15) divZ = 4, canFastDiv = 1;
	if(size == 31) divZ = 5, canFastDiv = 1;
	if(size == 63) divZ = 6, canFastDiv = 1;
	if(size == 127) divZ = 7, canFastDiv = 1;
	if(canFastDiv)
	{
		for(int N = 0; N < 3; ++N)
		{
			for(x=1;x<w;x++) for(y=0;y<h;y++)
				img[((x+w*y)<<2) + N]=(m*img[((x-1+w*y)<<2) + N]+img[((x+w*y)<<2) + N])>>divZ;
			for(x=w-2;x>=0;x--) for(y=0;y<h;y++)
				img[((x+w*y)<<2) + N]=(m*img[((x+1+w*y)<<2) + N]+img[((x+w*y)<<2) + N])>>divZ;
			for(x=0;x<w;x++) for(y=1;y<h;y++)
				img[((x+w*y)<<2) + N]=(m*img[((x-w+w*y)<<2) + N]+img[((x+w*y)<<2) + N])>>divZ;
			for(y=h-2;y>=0;y--) for(x=0;x<w;x++)
				img[((x+w*y)<<2) + N]=(m*img[((x+w+w*y)<<2) + N]+img[((x+w*y)<<2) + N])>>divZ;
		}
		return;
	}
	for(int N = 0; N < 3; ++N)
	{
		for(x=1;x<w;x++) for(y=0;y<h;y++)
			img[((x+w*y)<<2) + N]=(m*img[((x-1+w*y)<<2) + N]+img[((x+w*y)<<2) + N])/(m+1);
		for(x=w-2;x>=0;x--) for(y=0;y<h;y++)
			img[((x+w*y)<<2) + N]=(m*img[((x+1+w*y)<<2) + N]+img[((x+w*y)<<2) + N])/(m+1);
		for(x=0;x<w;x++) for(y=1;y<h;y++)
			img[((x+w*y)<<2) + N]=(m*img[((x-w+w*y)<<2) + N]+img[((x+w*y)<<2) + N])/(m+1);
		for(y=h-2;y>=0;y--) for(x=0;x<w;x++)
			img[((x+w*y)<<2) + N]=(m*img[((x+w+w*y)<<2) + N]+img[((x+w*y)<<2) + N])/(m+1);
	}
}

void fastBlur(uint8 *img, int w, int h, int size)
{
	int x,y;//,m;
        int m=size;
	int divZ = 0;
	int canFastDiv = 0;
	if(size == 1) divZ = 1, canFastDiv = 1;
	if(size == 3) divZ = 2, canFastDiv = 1;
	if(size == 7) divZ = 3, canFastDiv = 1;
	if(size == 15) divZ = 4, canFastDiv = 1;
	if(size == 31) divZ = 5, canFastDiv = 1;
	if(size == 63) divZ = 6, canFastDiv = 1;
	if(size == 127) divZ = 7, canFastDiv = 1;
	if(canFastDiv)
	{
			for(x=1;x<w;x++) for(y=0;y<h;y++)
				img[x+w*y]=(m*img[x-1+w*y]+img[x+w*y])>>divZ;
			for(x=w-2;x>=0;x--) for(y=0;y<h;y++)
				img[x+w*y]=(m*img[x+1+w*y]+img[x+w*y])>>divZ;
			for(x=0;x<w;x++) for(y=1;y<h;y++)
				img[x+w*y]=(m*img[x-w+w*y]+img[x+w*y])>>divZ;
			for(y=h-2;y>=0;y--) for(x=0;x<w;x++)
				img[x+w*y]=(m*img[x+w+w*y]+img[x+w*y])>>divZ;
		return;
	}
		for(x=1;x<w;x++) for(y=0;y<h;y++)
			img[x+w*y]=(m*img[x-1+w*y]+img[x+w*y])/(m+1);
		for(x=w-2;x>=0;x--) for(y=0;y<h;y++)
			img[x+w*y]=(m*img[x+1+w*y]+img[x+w*y])/(m+1);
		for(x=0;x<w;x++) for(y=1;y<h;y++)
			img[x+w*y]=(m*img[x-w+w*y]+img[x+w*y])/(m+1);
		for(y=h-2;y>=0;y--) for(x=0;x<w;x++)
			img[x+w*y]=(m*img[x+w+w*y]+img[x+w*y])/(m+1);
}



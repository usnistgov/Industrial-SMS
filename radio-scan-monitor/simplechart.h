#define uint8 unsigned char

int strEq(const char *inputText1, const char *inputText2)
{
	int x = 0;
	if(inputText1 == NULL && inputText2 == NULL) return 1;
	if(inputText1 == NULL || inputText2 == NULL) return 0;
	while(inputText1[x] != 0 && inputText2[x] != 0)
	{
		if(inputText1[x] != inputText2[x]) return 0;
		++x;
	}
	if(inputText1[x] != inputText2[x]) return 0;
	return 1;
}

class CSimpleChart
{
private:
	int dataSize;
	float *data;
	int dataPos;
	float scale;
	float mScale;
	float zeroV;
	float curMin;
	float curMax;
	int scalingType;
	int drawAxis;
	int inverted;
	int color;
	int axisColor;
	int DX, DY;
	int SX, SY;

	void updateScaling()
	{
		if(scalingType == 0)
		{
			if(scale != 0) mScale = 1.0 / scale;
			float dmin = data[0], dmax = dmin;
			for(int x = 0; x < dataSize; ++x)
			{
				if(data[x] > dmax) dmax = data[x];
				if(data[x] < dmin) dmin = data[x];
			}
			curMin = dmin;
			curMax = dmax;
		}
		if(scalingType == 1)
		{
			float dmin = data[0], dmax = dmin;
			for(int x = 0; x < dataSize; ++x)
			{
				if(data[x] > dmax) dmax = data[x];
				if(data[x] < dmin) dmin = data[x];
			}
			if(dmax == dmin) dmax = dmin+0.00001;
			curMin = dmin;
			curMax = dmax;
			float nzv = 0.5*(dmax+dmin);
			zeroV = nzv;//0.99*zeroV + 0.01*nzv;
			float sc = 1.3*(dmax - dmin);
			scale = sc;//0.99*scale + 0.01*sc;
			mScale = 1.0 / scale;
		}
		if(scalingType == 2)
		{
			float dmin = data[0], dmax = dmin;
			for(int x = 0; x < dataSize; ++x)
			{
				if(data[x] > dmax) dmax = data[x];
				if(data[x] < dmin) dmin = data[x];
			}
			curMin = dmin;
			curMax = dmax;
			zeroV = 0.5*(dmax+dmin);
		}
	};
	float rawVal(float normPos)
	{
		int stepsz = dataSize / SX;
		if(stepsz < 1) //interpolation: more graphical points than data
		{
			float rpos = normPos*dataSize;
			int i1 = rpos;
			int i2 = i1+1;
			float c2 = rpos - i1, c1 = i2-rpos;
			int ri1 = dataPos - i1; if(ri1 < 0) ri1 += dataSize;
			int ri2 = dataPos - i2; if(ri2 < 0) ri2 += dataSize;
			return data[ri1]*c1 + data[ri2]*c2;
		}
		else //averaging
		{
			float rpos = normPos*dataSize;
			int i1 = rpos - stepsz;// - stepsz/2;
//			if(i1 < 0) i1 = 0;
			int i2 = i1 + stepsz;
			int ri1 = dataPos - i1; if(ri1 < 0) ri1 += dataSize;
			int ri2 = dataPos - i2; if(ri2 < 0) ri2 += dataSize;
			if(ri1 < 0) return 0;
			if(ri2 < 0) return 0;
			float aV = 0;
			int minI = ri1, maxI = ri2;
			if(maxI < minI) maxI = ri1, minI = ri2;
			for(int n = minI; n <= maxI; n++)
				aV += data[n];
			aV /= (maxI - minI + 1);
			return aV;
		}
	}
	float normVal(float val)
	{
		return (val - zeroV) * mScale;
	};
public:
	CSimpleChart(int length)
	{
		dataSize = length;
		if(dataSize < 1) dataSize = 1;
		data = new float[dataSize];
		for(int x = 0; x < dataSize; ++x) data[x] = 0;
		dataPos = 0;

		scalingType = 0;
		zeroV = 0;
		scale = 1;
		drawAxis = 1;
		inverted = 0;
		color = 0xFF;
		axisColor = 0xFFFFFF;
	};
	~CSimpleChart()
	{
		delete[] data;
	};
	void setViewport(int dx, int dy, int sizeX, int sizeY)
	{
		DX = dx;
		DY = dy;
		SX = sizeX;
		SY = sizeY;
	};
	void setParameter(const char *name, const char *value)
	{
		if(strEq(name, "draw axis")) drawAxis = strEq(value, "yes");
		if(strEq(name, "inverted")) inverted = strEq(value, "yes");
		if(strEq(name, "scaling"))
		{
			if(strEq(value, "manual")) scalingType = 0; 
			if(strEq(value, "auto")) scalingType = 1; 
			if(strEq(value, "follow center")) scalingType = 2; 
		}
	};
	void setParameter(const char *name, float value)
	{
		if(strEq(name, "zero value")) zeroV = value;
		if(strEq(name, "scale")) scale = value;
	};
	void setParameter(const char *name, int r, int g, int b)
	{
		if(strEq(name, "axis color")) axisColor = (r<<16) + (g<<8) + b;
		if(strEq(name, "color")) color = (r<<16) + (g<<8) + b;
	};
	void clear()
	{
		for(int x = 0; x < dataSize; ++x) data[x] = 0;
	};
	void addV(float v)
	{
		data[dataPos++] = v;
		if(dataPos >= dataSize) dataPos = 0;
		return;
//		float v5 = median5_f();
//		int h = dataPos - 6;
//		if(h < 0) h += dataSize;
//		data[h] = v5;
//		updateScaling();
	};
	float getV(int hist_depth)
	{
		int p = dataPos - hist_depth;
		while(p < 0) p += dataSize;
		return data[p];
	};
	int val2color(float power_level)
	{
		int r, g, b;
		float blue_lvl = -80;
		float green_lvl = -60;
		float red_lvl = -20;
		if(power_level < blue_lvl) {r = 0; g = 0; b = 255;}
		else if(power_level < green_lvl)
		{
			float rel = (green_lvl - power_level) / (green_lvl - blue_lvl);
			g = 255 * (1.0 - rel);
			b = 255 - g;
			r = 0;
		}
		else if(power_level < red_lvl)
		{
			float rel = (red_lvl - power_level) / (red_lvl - green_lvl);
			r = 255 * (1.0 - rel);
			g = 255 - r;
			b = 0;
		}
		else {r = 255; g = 0; b = 0;}
		return (r<<16)|(g<<8)|b;
	};
	void draw(uint8* drawPix, int w, int h)
	{
		updateScaling();
		float mSX = 1.0 / (float)(SX-1);
		int r = color&0xFF;
		int g = (color>>8)&0xFF;
		int b = (color>>16)&0xFF;
		int prev_x = -1;
		int prev_y = -1;
		for(int x = 0; x < SX; ++x)
		{
			float rp = (float)x; rp *= mSX;
			float rawV = rawVal(rp);
			float v = normVal(rawV);
			int sp_color = val2color(rawV);
			if(v > 1) v = 1;
			if(v < 0) v = 0;
			int xx = DX + SX-1 - x;
			int yy = DY + SY - v*SY;
			if(inverted)
				yy = DY + v*SY;
//			int idx = (yy*w + xx);
//			if(x == 123) //debug
//				printf("x %d rp %g v %g xx %d yy %d\n", x, rp, v, xx, yy);
			if(xx >= 0 && xx < w && yy >= 0 && yy < h)
			{
				for(int yyy = yy; yyy < DY+SY; yyy++)
					((unsigned int*)drawPix)[yyy*w + xx] = sp_color;
				if(prev_x >= 0 && prev_x < w && prev_y >= 0 && prev_y < h)
					grp_drawLN(drawPix, w, h, prev_x, prev_y, xx, yy, r, g, b);
			}
			prev_x = xx;
			prev_y = yy;
		}
		if(drawAxis)
		{
			for(int x = 0; x < SX; ++x)
			{
				int xx = DX + x;
				int yy = DY + SY-1;
				int idx = (yy*w + xx);
				if(xx >= 0 && xx < w && yy >= 0 && yy < h)
					((unsigned int*)drawPix)[idx] = axisColor;
			}
			for(int y = 0; y < SY; ++y)
			{
				int xx = DX;
				int yy = DY + y;
				int idx = (yy*w + xx);
				if(xx >= 0 && xx < w && yy >= 0 && yy < h)
					((unsigned int*)drawPix)[idx] = axisColor;
			}
		}
	};
	float getMin() {return curMin;};
	float getMax() {return curMax;};
	int getValueY(float val)
	{
		float v = normVal(val);
//		if(v > 1) v = 1;
//		if(v < 0) v = 0;
		int yy = DY + SY - v*SY;
		if(inverted)
			yy = DY + v*SY;
		return yy;
	};
	int getX() {return DX;};
	int getY() {return DY;};
	int getSizeX() {return SX;};
	int getSizeY() {return SY;};
	int getDataSize() {return dataSize;};
	float getMean()
	{
		float res = 0;
		for(int x = 0; x < dataSize; ++x) res += data[x];
		float z = dataSize;
		z += (dataSize == 0);
		return res/z;
	};
	float getSDV()
	{
		float mean = getMean();
		float res = 0;
		for(int x = 0; x < dataSize; ++x)
		{
			float dd = (data[x] - mean);
			res += dd*dd;
		}
		float z = dataSize;
		z += (dataSize == 0);
		return sqrt(res/z);
	};
};


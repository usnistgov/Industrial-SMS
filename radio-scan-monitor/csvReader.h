#ifndef CSVREADER__H
#define CSVREADER__H

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

//#include <dos.h>


/*
matchTBuf(char *it1, int it1l, char *it2, int it2l, int p1m) - match sequence it1 with it2 with mistypes, p1m allows deleted character
compStr(char *inputText1, char *inputText2) - compares texts

csvReader:
csvReader(char *fnm) - init by filename
csvReader(int handle) - init by opened file handle
csvReader(char *buf, int length) - init by buffer with given length
char *getBuf() - get raw buffer
int readInt() - read next integer, puts cursor to the next non-digit character
double readDouble() - reads floating point (including 'e' or 'E' character afterwards)
char readChar() - reads one character, puts cursor one position forward
char *readString() - skips to next letter, reads sequence of letters (numbers are considered as string end) up to 1000 characters length
char *readNumString() -  skips to next letter/number, reads sequence of letters and numbers up to 1000 characters length
int readNextLine(char *ret) - reads whole line into ret, returns line length
void skipNext() - skips any separators and one alphanumerical sequence after the cursor
int gotoLine(int num)
void nextLine() - skip to the next line
int getLinesCount() 
int getPosition()  - get current cursor position
int getCurLine() 
void gotoBegin() - set cursor to the beginning of the file
void setPosition(int pos) - set specific cursor position
int getFLength()  - get file/buffer length
int seekString(char *str) - returns position of closest str entry, position of first character of str entry is returned, cursor moved to item position
int getIntByName(char *name) - read first integer in the file, surrounded by <name> </name> sequence
double getDoubleByName(char *name) - read first double in the file, surrounded by <name> </name> sequence
char *getStringByName(char *name) - read first string in the file, surrounded by <name> </name> sequence


*/

int isMSep(char c)
{
	if(c == ' ')
		return 1;
	return 0;
}

int matchTBuf(char *it1, int it1l, char *it2, int it2l, int p1m)
{
	if(it1l < 2 || it2l < 2) return -1;
	char *t1 = new char[it1l];
	char *t2 = new char[it2l];
	int *skips = new int[it1l];
	int t1l = 0, t2l = 0, tlc = 0;
	skips[0] = 0;
	for(int x = 0; x < it1l; ++ x)
	{
		if(t1l > 0 && tlc==1)
			skips[t1l] = skips[t1l-1];
		tlc = 0;
		if(!isMSep(it1[x]))
		{
			t1[t1l] = it1[x];
			++t1l;
			tlc = 1;
		}
		else
			skips[t1l]++;
	}
	for(int x = 0; x < it2l; ++ x)
	{
		if(!isMSep(it2[x]))
		{
			t2[t2l] = it2[x];
			++t2l;
		}
	}
	int mRes = 0;
	for(int x = 0; x < t1l-t2l; ++ x)
	{
		mRes = 0;
		int p1 = 0;
		for(int y = 0; y < t2l; ++ y)
			if(t1[x+y+p1] != t2[y])
			{
				if(p1 == 0 && t1[x+y+1] == t2[y] && p1m>0)
					p1 = 1;
				++mRes;
			}
		if(mRes < 1 + p1*(p1m>0) + t2l/7)
		{
			delete t1;
			delete t2;
//			end[0] = x + t2l + skips[x+t2l];
			return x + skips[x];
		}
	}
	delete t1;
	delete t2;
	delete skips;
	return -1;
}
int compStr(char *inputText1, char *inputText2)
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
int compSubStr(char *inputText1, char *inputText2)
{
	int x = 0;
	if(inputText1 == NULL && inputText2 == NULL) return 1;
	if(inputText1 == NULL || inputText2 == NULL) return 0;
	while(inputText1[x] != 0 && inputText2[x] != 0)
	{
		if(inputText1[x] != inputText2[x]) return 0;
		++x;
	}
	return 1;
}

int sign(double x)
{
	if(x >= 0) return 1;
	else return -1;
}
double frandom(double from, double to)
{
	double a = rand()%32768;
	double k = a / 32768.0;
	return from*k + to*(1.0-k);
}
double frand01()
{
	return frandom(0, 1);
}

class csvReader
{
private:
	bool active;
	int fileHandle;
	int curLine;
	int totalLines;
	int fileLength;
	int position;
	char *buff;
	char *retBuf;

	int char2digit(char c)
	{
	    return (c - 48);
	};
	bool isSeparator(char c)
	{
		if(c == ';') return true;
		if(c == 13) return true;
		if(c == 10)
		{
			++curLine;
			return true;
		}
		if(c == ' ') return true;
		if(c == '/') return true;
		if(c == '(') return true;
		if(c == ')') return true;
		if(c == '<') return true;
		if(c == '>') return true;
		if(c == ':') return true;
		if(c == '.') return true;
		if(c == ',') return true;
		if(c == '\t') return true;
		return false;
	};
	bool isNotNumber(char c)
	{
		if(c == ';') return true;
		if(c == 13) return true;
		if(c == 10)
		{
			++curLine;
			return true;
		}
		if(c == '-') return false;
		if(c == '+') return false;
		if(c >= '0' && c <= '9') return false;
		return true;
	};
	bool isNotChar(char c)
	{
		if(c == ';') return true;
		if(c == 13) return true;
		if(c == 10)
		{
			++curLine;
			return true;
		}
		if(c >= 'a' && c <= 'z') return false;
		if(c >= 'A' && c <= 'Z') return false;
		return true;
	};
//	bool isNotSymbol(char c);
	bool eof(int x)
	{
	    if(x < fileLength - 1) return false;
	    return true;
	};

	int skipComment(int x);
	void initByHandle(int handle)
	{
		if(handle != -1)
			active = true;
		fileHandle = handle;
		lseek(fileHandle,0,0);
		fileLength = lseek(fileHandle,0,2);
		lseek(fileHandle,0,0);

	//	buff = new char[fileLength+1];
	//	retBuf = new char[1024];
		buff = (char*)malloc(fileLength+1);
		retBuf = (char*)malloc(1024);
		if(read(fileHandle, buff, fileLength) != fileLength)
		{
	//		ShowMessage("file reading gluk");
			printf("problems reading file\n");
			active = false;
		}
		curLine = 0;
		totalLines = 0;
		position = 0;
		for(int x = 0; x < fileLength; ++ x)
			if(buff[x] == 10) ++totalLines;
	};
public:
	csvReader(int handle)
	{
		initByHandle(handle);
	};
	csvReader(char *fnm)
	{
		int fr = open(fnm, O_RDONLY);
		initByHandle(fr);
		close(fr);
	}

	csvReader(char *buf, int length)
	{
		if(buf != NULL)
			active = true;
		fileLength = length;

	//	buff = new char[fileLength+1];
	//	retBuf = new char[1024];
		buff = (char*)malloc(fileLength+1);
		retBuf = (char*)malloc(1024);
		for(int x = 0; x < fileLength; ++ x)
			buff[x] = buf[x];
		curLine = 0;
		totalLines = 0;
		position = 0;
		for(int x = 0; x < fileLength; ++ x)
			if(buff[x] == 10) ++totalLines;
	};
	~csvReader()
	{
	//	delete retBuf;
	//	delete buff;
		free(buff);
		free(retBuf);
	};
	char *getBuf() {return buff;};
	int readInt()
	{
		int x = position;
		int ret = 0;
		int sgn = 1;
		while(isNotNumber(buff[x]) && !eof(x)) ++x;
		while(!isNotNumber(buff[x]) && !eof(x)){
		ret *= 10;
		if(buff[x] == '-')
			sgn = -1;
		else if(buff[x] == '+')
			sgn = 1;
		else
			ret += char2digit(buff[x]);
			++x;
		}
		while(isSeparator(buff[x]) && !eof(x)) ++x;
		position = x;
		return ret*sgn;
	};
	long long readLong()
	{
		int x = position;
		long long ret = 0;
		long long sgn = 1;
		while(isNotNumber(buff[x]) && !eof(x)) ++x;
		while(!isNotNumber(buff[x]) && !eof(x)){
		ret *= 10;
		if(buff[x] == '-')
			sgn = -1;
		else
			ret += char2digit(buff[x]);
			++x;
		}
		while(isSeparator(buff[x]) && !eof(x)) ++x;
		position = x;
		return ret*sgn;
	};
	double readDouble()
	{
		int x = position;
		double bpoint = 0;
		double apoint = 0;
		double div = 1;
		double sgn = 1;
		double ret;
		while(isNotNumber(buff[x]) && !eof(x)) ++x;
		while(!isNotNumber(buff[x]) && !eof(x)){
			bpoint *= 10.0;
			if(buff[x] == '-')
				sgn = -1;
			else
				bpoint += (double)char2digit(buff[x]);
				++x;
		}
		if(buff[x] == '.')
		{
			++x;
			while(!isSeparator(buff[x]) && isNotChar(buff[x]) && !eof(x)){
				apoint *= 10.0;
				div *= 10.0;
				apoint += (double)char2digit(buff[x]);
				++x;
			}
		}
		int exp = 0;
		if(buff[x] == 'E' || buff[x] == 'e')
		{
			position = x;
			exp = readInt();
		}
		else
		{
			while(isSeparator(buff[x]) && !eof(x)) ++x;
			position = x;
		}
		double bp = bpoint, ap = apoint, dv = div;
		ret = bp + ap/dv;
		ret *= pow(10, exp);
		return ret*sgn;
	};
	char readChar()
	{
		int x = position;
		while(isNotChar(buff[x]) && !eof(x)) ++x;
		position = x+1;
		return buff[x];
	};
	char *readString()
	{
		int x = position;
		int rpos = 0;
		while(isNotChar(buff[x]) && !eof(x)) ++x;
		while(!isNotChar(buff[x]) && !eof(x) && rpos < 1000)
		{
			retBuf[rpos] = buff[x];
			++x;
			++rpos;
		}
		retBuf[rpos] = 0;
		position = x;
		return retBuf;
	};
	char *readNumString()
	{
		int x = position;
		int rpos = 0;
		while(isNotChar(buff[x]) && isNotNumber(buff[x]) && !eof(x)) ++x;
		while((!isNotChar(buff[x]) || !isNotNumber(buff[x])) && !eof(x) && rpos < 1000)
		{
			retBuf[rpos] = buff[x];
			++x;
			++rpos;
		}
		retBuf[rpos] = 0;
		position = x;
		return retBuf;
	};
	int readNextLine(char *ret)
	{
		int x = position, bgpos = 0;
		while(x > 0 && buff[x] != 10) --x;
		while((buff[x] == 13 || buff[x] == 10) && !eof(x)) ++x;
		bgpos = x;
		while(buff[x] != 10  && !eof(x))
		{
			ret[x - bgpos] = buff[x];
			++x;
		}
		position = x+1;
		return position - bgpos - 1;
	};
	void skipNext()
	{
		int x = position+1;
		if(eof(x)) return;
		while(isSeparator(buff[x]) && !eof(x)) ++x;
		while(!isSeparator(buff[x]) && !eof(x)) ++x;
		while(isSeparator(buff[x]) && !eof(x)) ++x;
		position = x;
		return;
	};
	int gotoLine(int num)
	{
		if(num >= totalLines || num < 0) return -1;
		if(curLine > num){
			position = 0;
			curLine = 0;
		}
		while(curLine < num)
			nextLine();
		return curLine;
	};
	void nextLine()
	{
		int x = position;
		while(buff[x] != 10  && !eof(x)) ++x;
		while(isSeparator(buff[x]) && !eof(x)) ++x;
		position = x;
	};
	int getLinesCount() {return totalLines;};
	int getPosition() {return position;};
	int getCurLine() {return curLine;};
	void gotoBegin() {position = 0;};
	void setPosition(int pos) {if(pos < 0) pos = 0; if(pos > fileLength-1) pos = fileLength-1; position = pos;};
	int getFLength() {return fileLength;};
	int seekString(char *str)
	{
		int x = position;
		int slng = 0;
		int isGood = 0;
		while(str[slng] != 0) ++slng;
		if(slng == 0) return -1;
		while(!eof(x))
		{
			if(buff[x] == str[0])
			{
				if(slng == 1)
					isGood = 1;
				else
					isGood = 0;
				if(slng > 1 && buff[x+1] == str[1])
				{
					isGood = 1;
					for(int z = 2; z < slng; ++ z)
					{
						if((x+z > fileLength-1)
						 || (buff[x+z] != str[z]))
						{
							isGood = 0;
							break;
						}
					}
				}
			}
			if(isGood)
			{
				position = x+1;
				return position;
			}
			++x;
		}
		return -1;
	};
	int getIntByName(char *name)
	{
		char bname[256];
		char ename[256];
		bname[0] = '<';
		ename[0] = '<';
		ename[1] = '/';
		int x = 0;
		while(name[x] != 0 && x < 250)
		{
			bname[x+1] = name[x];
			ename[x+2] = name[x];
			++x;
		}
		bname[x+1] = '>';
		ename[x+2] = '>';
		bname[x+2] = 0;
		ename[x+3] = 0;
		int tmppos = position;
		gotoBegin();
		int bgPos = seekString(bname);
		int edPos = seekString(ename);
		position = tmppos;
		if(bgPos < 0 || edPos < 0 || edPos < bgPos) return -1;
		position = bgPos+1;
		int ret = readInt();
		if(position > edPos+2) return -1;
		else return ret;
	};
	double getDoubleByName(char *name)
	{
		char bname[256];
		char ename[256];
		bname[0] = '<';
		ename[0] = '<';
		ename[1] = '/';
		int x = 0;
		while(name[x] != 0 && x < 250)
		{
			bname[x+1] = name[x];
			ename[x+2] = name[x];
			++x;
		}
		bname[x+1] = '>';
		ename[x+2] = '>';
		bname[x+2] = 0;
		ename[x+3] = 0;
		int tmppos = position;
		gotoBegin();
		int bgPos = seekString(bname);
		int edPos = seekString(ename);
		position = tmppos;
		if(bgPos < 0 || edPos < 0 || edPos < bgPos) return -1;
		position = bgPos+1;
		double ret = readDouble();
		if(position > edPos+2) return -1;
		else return ret;
	};
	char *getStringByName(char *name)
	{
		char bname[256];
		char ename[256];
		bname[0] = '<';
		ename[0] = '<';
		ename[1] = '/';
		int x = 0;
		while(name[x] != 0 && x < 250)
		{
			bname[x+1] = name[x];
			ename[x+2] = name[x];
			++x;
		}
		bname[x+1] = '>';
		ename[x+2] = '>';
		bname[x+2] = 0;
		ename[x+3] = 0;
		int tmppos = position;
		gotoBegin();
		int bgPos = seekString(bname);
		int edPos = seekString(ename);
		position = tmppos;
		if(bgPos < 0 || edPos < 0 || edPos < bgPos) return NULL;
		position = bgPos+1;
		gotoBegin();
		seekString(bname);
		int bsPos = seekString((char*)">");
		char *ret = new char[edPos - bsPos];
		for(int x = bsPos; x < edPos-1; ++ x)
			ret[x-bsPos] = buff[x];
		ret[edPos-1 - bsPos] = 0;
		return ret;
	};
};

#endif

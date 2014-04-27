#pragma once

#ifndef _H_FP
#define _H_FP

#include "stdafx.h"
#include "defines.h"

volatile USHORT floppyState = FLOPPY_STATE_NO_MEDIA;
volatile USHORT floppyError = FLOPPY_ERROR_NONE;
int track;
string floppyPath;
volatile USHORT throwItrpt = 0;

struct FloppyInfo
{
	USHORT floppyVersion;
	USHORT floppyFlags;
	USHORT unuse[512 - 2];
};

HANDLE threadH = NULL;
DWORD threadID = 0;

void setState(USHORT _state)
{
	floppyState = _state;
	if (throwItrpt != 0)
		(*additr)(throwItrpt);
}

void setError(USHORT _error)
{
	floppyError = _error;
	if (throwItrpt != 0)
		(*additr)(throwItrpt);
}

int LoadDisk(string _path)
{
	floppyState = FLOPPY_STATE_NO_MEDIA;
	int ret = 0;
	fstream floppyFile(_path, ios::in | ios::binary);
	if (!(!floppyFile))
	{
		ret = -1;
		return -1;
	}
	FloppyInfo *inf = new FloppyInfo;
	floppyFile.read((char *)(inf), sizeof(FloppyInfo) / sizeof(char));
	USHORT flVer = inf->floppyVersion;
	BYTE flVerHi = flVer >> 8;
	if (flVerHi > FLOPPY_VER_HI)
	{
		ret = -2;
		goto _ld_end;
	}
	else if ((flVerHi == FLOPPY_VER_HI) && ((flVer & 0xFF) > FLOPPY_VER_LO))
	{
		ret = -2;
		goto _ld_end;
	}
	USHORT flFlag = inf->floppyFlags;
	if (flFlag&FLOPPY_INFO_FLAG_PROTECTED)
	{
		floppyState = FLOPPY_STATE_READY_WP;
	}
	_ld_end:floppyFile.close();
	if (ret == 0)
		floppyPath = _path;
	return ret;
}

DWORD WINAPI FloppyThreadRead(LPVOID lpParam)
{
	USHORT pastState = floppyState;
	setState(FLOPPY_STATE_BUSY);
	ULONGLONG itr = *(ULONGLONG*)(lpParam);
	USHORT x = (USHORT)(itr >> 16), y = (USHORT)(itr);
	if (x >= 80 * 18)
	{
		setError(FLOPPY_ERROR_BAD_SECTOR);
		setState(pastState);
		return 0;
	}
	fstream file(floppyPath, ios::in | ios::binary);
	file.seekg(sizeof(FloppyInfo) / sizeof(char), ios::beg);
	{
		int _track = x / 18;
		int trackSeekingTime = (int)((double)(2.4) * abs(track - _track) * 1000) / CLOCKS_PER_SEC;
		track = _track;
		clock_t start = clock();
		while (clock() - start < trackSeekingTime);
	}
	int i, j;
	USHORT data;
	for (i = 0, j = y; i < 512; i++, j++)
	{
		file.read((char *)(&data), sizeof(USHORT) / sizeof(char));
		(*setMem)(j, data);
	}
	file.close();
	setState(pastState);
	return 0;
}

DWORD WINAPI FloppyThreadWrite(LPVOID lpParam)
{
	USHORT pastState = floppyState;
	setState(FLOPPY_STATE_BUSY);
	ULONGLONG itr = *(ULONGLONG*)(lpParam);
	USHORT x = (USHORT)(itr >> 16), y = (USHORT)(itr);
	if (x >= 80 * 18)
	{
		setError(FLOPPY_ERROR_BAD_SECTOR);
		setState(pastState);
		return 0;
	}
	fstream file(floppyPath, ios::out | ios::binary);
	file.seekp(sizeof(FloppyInfo) / sizeof(char), ios::beg);
	{
		int _track = x / 18;
		int trackSeekingTime = (int)((double)(2.4) * abs(track - _track) * 1000) / CLOCKS_PER_SEC;
		track = _track;
		clock_t start = clock();
		while (clock() - start < trackSeekingTime);
	}
	int i, j;
	USHORT data;
	for (i = 0, j = y; i < 512; i++, j++)
	{
		(*getMem)(j, &data);
		file.write((char *)(&data), sizeof(USHORT) / sizeof(char));
	}
	file.close();
	setState(pastState);
	return 0;
}

#endif
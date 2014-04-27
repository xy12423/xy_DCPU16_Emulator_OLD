#pragma once

#ifndef _H_MAIN
#define _H_MAIN

#define unitLen 4

#include "defines.h"
#include "stdafx.h"

struct color
{
	GLfloat r, g, b;
};

color mem[128][96];

GLvoid myDisplay(GLvoid)
{
	int i, j;
	glLoadIdentity();
	glOrtho(0, 128, -96, 0, -10.0f, 10.0f);
	for (i = 0; i < 128; i++)
		for (j = 0; j < 96; j++)
		{
			glColor3f(mem[i][j].r, mem[i][j].g, mem[i][j].b);
			glRectf((GLfloat)(i), (GLfloat)(-j), (GLfloat)(i + 1), (GLfloat)(-j - 1));
		}
	glFlush();
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	glutReshapeWindow(128 * unitLen, 96 * unitLen);
	glOrtho(0, 128, -96, 0, -10.0f, 10.0f);
}

USHORT mf[256];
color mp[16];
USHORT
mfDef[256],
mpDef[16] = {
	0x0000, 0x0800, 0x0080, 0x0008, 0x0880, 0x0808, 0x0088, 0x0CCC,
	0x0888, 0x0F00, 0x00F0, 0x000F, 0x0FF0, 0x0F0F, 0x00FF, 0x0FFF,
};
volatile USHORT mvStart = 0, mfStart = 0, mpStart = 0;
volatile bool usrMF = false, usrMP = false;
volatile int mOn = 0;
volatile int blinkCount = 0;
volatile bool blinkClock = false;

#ifdef _DEBUG
bool refed[128][96];
#endif

void CALLBACK ClockMain(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	int i, x, y;
	if (mOn == 0)
		return;
	else if (mOn == 1)
	{
		mOn = 2;
		clock_t start = clock();
		while (clock() - start < CLOCKS_PER_SEC);
		return;
	}
	blinkCount++;
	if (blinkCount >= 1000)
		blinkClock = !blinkClock;
	if (usrMF)
	{
		for (i = 0; i < 256; i++)
			(*getMem)((USHORT)(i + mfStart), mf + i);
	}
	else
	{
		for (i = 0; i < 256; i++)
			mf[i] = mfDef[i];
	}
	if (usrMP)
	{
		USHORT cl = 0;
		for (i = 0; i < 16; i++)
		{
			(*getMem)((USHORT)(i + mpStart), &cl);
			mp[i].r = (GLfloat)((cl & 0x0F00) >> 8) / 15.0f;
			mp[i].g = (GLfloat)((cl & 0x00F0) >> 4) / 15.0f;
			mp[i].b = (GLfloat)(cl & 0x000F) / 15.0f;
		}
	}
	else
	{
		USHORT cl = 0;
		for (i = 0; i < 16; i++)
		{
			cl = mpDef[i];
			mp[i].r = (GLfloat)((cl & 0x0F00) >> 8) / 15.0f;
			mp[i].g = (GLfloat)((cl & 0x00F0) >> 4) / 15.0f;
			mp[i].b = (GLfloat)(cl & 0x000F) / 15.0f;
		}
	}
	USHORT cel = 0;
	color fore, back;
	BYTE chr;
	bool blink;
	UINT font;
	int l, n;
	i = 0;
	for (y = 0; y < 96; y += 8)
	{
		for (x = 0; x < 128; x += 4)
		{
			(*getMem)((USHORT)(i + mvStart), &cel);
			fore = mp[(cel & 0xF000) >> 12];
			back = mp[(cel & 0x0F00) >> 8];
			blink = cel & 0x0080;
			if (blink && blinkClock)
			{
				back.r = 1.0f - back.r;
				back.g = 1.0f - back.g;
				back.b = 1.0f - back.b;
			}
			chr = cel & 0x007F;
			font = (mf[chr * 2] << 16) + mf[chr * 2 + 1];
			for (l = x + 3; l >= x; l--)
				for (n = y; n < y + 8; n++)
				{
					if (font & 0x0001)
						mem[l][n] = fore;
					else
						mem[l][n] = back;
					font = font >> 1;
				}
			i++;
		}
	}
	glutPostRedisplay();
	return;
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(128 * unitLen, 96 * unitLen);
	int argc = 0;
	char argv[] = "\0";
	char *argv2[] = { argv };
	glutInit(&argc, argv2);
	glutCreateWindow("Monitor");
	glutReshapeFunc(ReSizeGLScene);
	glutDisplayFunc(&myDisplay);
	//glutTimerFunc(20, &ClockMain, 0);
	timeSetEvent(20, 1, &ClockMain, 0, TIME_PERIODIC);
	//glutIdleFunc(&ClockMain);
	while (1)
	{
		glutMainLoopEvent();
		Sleep(1);
	}
	return 0;
}

#endif
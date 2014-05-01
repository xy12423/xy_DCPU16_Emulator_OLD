#pragma once

#ifndef _H_MAIN
#define _H_MAIN

#define unitLen 4

#include "defines.h"
#include "stdafx.h"

HDC			hDC = NULL;
HGLRC		hRC = NULL;
HWND		hWnd = NULL;
HINSTANCE	hInstance;

bool	keys[256];
BOOL	active = TRUE;

//Keyboard



//Monitor

struct color
{
	GLfloat r, g, b;
};

color mem[128][96];

BOOL DrawGLScene(GLvoid)
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
	SwapBuffers(hDC);
	return TRUE;
}

int InitGL(GLvoid)
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	return TRUE;
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	glOrtho(0, 128, -96, 0, -10.0f, 10.0f);
}

GLvoid KillGLWindow(GLvoid)
{
	if (hRC)
	{
		if (!wglMakeCurrent(NULL, NULL))
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);

		if (!wglDeleteContext(hRC))
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hRC = NULL;
	}

	if (hDC && !ReleaseDC(hWnd, hDC))
	{
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;
	}

	if (hWnd && !DestroyWindow(hWnd))
	{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;
	}

	if (!UnregisterClass("Monitor", hInstance))
	{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;
	}
}

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;
	WNDCLASS	wc;
	DWORD		dwExStyle;
	DWORD		dwStyle;
	RECT		WindowRect;
	WindowRect.left = (long)0;
	WindowRect.right = (long)width;
	WindowRect.top = (long)0;
	WindowRect.bottom = (long)height;

	hInstance = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "Monitor";

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_DLGFRAME;

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		"Monitor",
		title,
		dwStyle |
		WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN,
		200, 200,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC = GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_ACTIVATE:
			if (!HIWORD(wParam))
				active = TRUE;
			else
				active = FALSE;
			return 0;
		case WM_SYSCOMMAND:							// Intercept System Commands
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
					return 0;							// Prevent From Happening
			}
			break;									// Exit
		case WM_CLOSE:								// Did We Receive A Close Message?
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		case WM_KEYDOWN:							// Is A Key Being Held Down?
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		case WM_KEYUP:								// Has A Key Been Released?
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		case WM_SIZE:								// Resize The OpenGL Window
			ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		case 0xFFFF:
			DrawGLScene();
			return 0;
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

USHORT mf[256];
color mp[16];
USHORT mfDef[256] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x7F41, 0x7F00, 0x427F, 0x4000, 0x7949, 0x4F00, 0x4949, 0x7F00,
	0x0F08, 0x7F00, 0x4F49, 0x7900, 0x7F49, 0x7900, 0x0101, 0x7F00,
	0x7F49, 0x7F00, 0x4F49, 0x7F00, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x7817, 0x7800, 0x7F49, 0x3600, 0x3E41, 0x4100,
	0x7F41, 0x3E00, 0x7F49, 0x4900, 0x7F09, 0x0900, 0x3E41, 0x7100,
	0x7F08, 0x7F00, 0x417F, 0x4100, 0x417F, 0x0100, 0x7F18, 0x6700,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
},
//Font is made by SuperSuperSea(Bobby825)
mpDef[16] = {
	0x0000, 0x0800, 0x0080, 0x0008, 0x0880, 0x0808, 0x0088, 0x0CCC,
	0x0888, 0x0F00, 0x00F0, 0x000F, 0x0FF0, 0x0F0F, 0x00FF, 0x0FFF,
};
volatile USHORT mvStart = 0, mfStart = 0, mpStart = 0;
volatile bool usrMF = false, usrMP = false;
volatile int mOn = 0;
volatile int blinkCount = 0;
volatile bool blinkClock = false;

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
	{
		blinkClock = !blinkClock;
		blinkCount = 0;
	}
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
			if (cel & 0x0080)
				blink = true;
			else
				blink = false;
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
	PostMessage(hWnd, 0xFFFF, 0, 0);
	return;
}

volatile BOOL done = FALSE;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	MSG msg;

	if (!CreateGLWindow("Monitor", 128 * unitLen, 96 * unitLen, 16, false))
		return 0;

	DrawGLScene();

	while (!done)
	{
		if (GetMessage(&msg, NULL, 0, 0))	// Is There A Message Waiting?
		{
			if (msg.message == WM_QUIT)
				done = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	KillGLWindow();
	return (msg.wParam);
}

#endif
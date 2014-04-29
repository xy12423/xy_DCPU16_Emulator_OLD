#pragma once

#ifndef _H_MAIN
#define _H_MAIN

#include "defines.h"
#include "stdafx.h"

USHORT dpState, dpError;

struct line
{
	GLfloat x1, y1, z1;
	GLfloat x2, y2, z2;
	GLfloat r, g, b;
};

line mem[128];
volatile USHORT mvStart = 0, mvEnd = 0, mvCount = 0;
volatile USHORT rotAngle = 0;

GLvoid myDisplay(GLvoid)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -10.0f);
	line t;
	for (int i = 0; i < mvCount; i++)
	{
		t = mem[i];
		glColor3f(t.r, t.g, t.b);
		glBegin(GL_LINES);
		glVertex3f(t.x1, t.y1, t.z1);
		glVertex3f(t.x2, t.y2, t.z2);
		glEnd();
	}
	glFlush();
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	if (height == 0)
		height = 1;

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

GLfloat mcr[] = {
	0.0001f, 0.5f, 0.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f
};
GLfloat mcg[] = {
	0.0001f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f
};
GLfloat mcb[] = {
	0.0001f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f
};

void CALLBACK ClockMain(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	int i, j;
	if (mvCount == 0)
		return;
	UINT cell;
	USHORT data;
	BYTE color;
	line t;
	(*getMem)(mvEnd - 2, &data);
	cell = data;
	(*getMem)(mvEnd - 1, &data);
	cell = (cell << 16) + data;
	t.y1 = (GLfloat)(cell >> 24);
	t.x1 = (GLfloat)((cell >> 16) & 0xFF);
	t.z1 = (GLfloat)(cell & 0xFF);
	color = (BYTE)(cell >> 8);
	t.r = mcr[color];
	t.g = mcg[color];
	t.b = mcb[color];
	for (i = mvStart, j = 0; i < mvEnd; i += 2, j++)
	{
		(*getMem)(i, &data);
		cell = data;
		(*getMem)(i + 1, &data);
		cell = (cell << 16) + data;
		t.y2 = (GLfloat)(cell >> 24);
		t.x2 = (GLfloat)((cell >> 16) & 0xFF);
		t.z2 = (GLfloat)(cell & 0xFF);
		mem[j] = t;
		t.x1 = t.x2;
		t.y1 = t.y2;
		t.z1 = t.z2;
		color = (BYTE)(cell >> 8);
		t.r = mcr[color];
		t.g = mcg[color];
		t.b = mcb[color];
	}
	glutPostRedisplay();
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(500, 500);
	int argc = 0;
	char argv[] = "\0";
	char *argv2[] = { argv };
	glutInit(&argc, argv2);
	glutCreateWindow("3D Vector Display");
	glutReshapeFunc(ReSizeGLScene);
	glutDisplayFunc(&myDisplay);
	//glutTimerFunc(20, &ClockMain, 0);
	timeSetEvent(20, 1, &ClockMain, 0, TIME_PERIODIC);
	//glutIdleFunc(&ClockMain);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	while (1)
	{
		glutMainLoopEvent();
		Sleep(1);
	}
	return 0;
}

#endif
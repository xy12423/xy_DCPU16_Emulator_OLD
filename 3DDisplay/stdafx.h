// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <Windows.h>
#include <MMSystem.h>
#pragma comment(lib, "winmm.lib")


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�

#include <gl\gl.h>
#include <gl\glu.h>
#ifndef _DEBUG
#pragma comment(lib,"freeglut_static_dll.lib")
#endif
#include <gl\freeglut.h>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

#include <ctime>
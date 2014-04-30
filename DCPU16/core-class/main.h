#pragma once

#ifndef _H_EMU_CLS_MAIN
#define _H_EMU_CLS_MAIN

#include "define.h"

class DCPU16
{
public:
	DCPU16();

	int doCode();
	void doCodeThread();

	int __cdecl setMem(USHORT add, USHORT dat);
	int __cdecl getMem(USHORT add, USHORT *dat);
	int __cdecl setReg(USHORT _reg, USHORT dat);
	int __cdecl getReg(USHORT _reg, USHORT *dat);
	int __cdecl additr(USHORT _itr);
private:
	//Global var
	USHORT mem[0x10000];
	USHORT reg[8];
	UINT pc;
	bool pcOf = false;
	USHORT sp, ex, ia;
	USHORT itr[256];
	unsigned char itrp = 0, itre = 0;
	bool itri = true;
	hardware hwt[65536];
	UINT hwn = 0;

	//Switcher
	int reta(USHORT a, USHORT &ret);
	int retb(USHORT b, USHORT &ret);
	int setb(USHORT b, USHORT dat, int shift);

	//Main functions
	int doL1(opcode code);
	int doL2(opcode code);
	int doL3();

	//Other functions
	opcode US2OP(USHORT code);
	USHORT OP2US(opcode code);
	void skipsingle();
	int skip();
	int popitr(USHORT &_itr);
};

#include "switch.h"
#include "function.h"
#include "export.h"
#include "emulator.h"

#endif
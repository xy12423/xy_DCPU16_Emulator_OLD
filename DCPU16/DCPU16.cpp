#include "stdafx.h"
#include "core/main.h"
using namespace std;

#define INS_RET_LEN 65536

USHORT m_ret[INS_RET_LEN];
string m_arg[INS_RET_LEN];

#ifdef _P_WIN
DWORD_PTR zero = 0;
MMRESULT timerH = 0;
void CALLBACK timer(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	cycleAll += 100;
	if (!doCodeB)
		timeKillEvent(timerH);
}
#endif
#ifdef _P_LIN
#include <ctime>
#include <sys/time.h>
void timer()
{
	while (doCodeB)
	{
		cycleAll += 100;
		usleep(1000);
	}
}
#endif
#ifdef _P_NA
#include <ctime>
#if CLOCKS_PER_SEC < 1000
#error You need a better clock
#endif
void timer()
{
	clock_t start;
	while (doCodeB)
	{
		start = clock();
		cycleAll += 100;
		while (clock() - start < 1);
	}
}
#endif

void clockStarter()
{
#ifdef _P_WIN
	timerH = timeSetEvent(1, 1, timer, zero, TIME_PERIODIC);
#endif
#if defined(_P_LIN) || defined (_P_NA)
	thread tmr(timer);
	tmr.detach();
#endif
}

void assm(string arg)
{
	static USHORT add = 0;
	char in[50];
	string ins;
	if (arg != "")
		add = toNum(arg, 2);
	while (true)
	{
		printf("%04X:", add);
		cin.getline(in, 50, '\n');
		ins = in;
		if (ins.length() == 0)
			break;
		int len = assembler(ins, m_ret, INS_RET_LEN);
		switch (len)
		{
			case _ERR_ASM_NOT_SUPPORTED:
				cout << "Instruction is not supported" << endl;
				break;
			case _ERR_ASM_ILLEGAL:
			case _ERR_ASM_ILLEGAL_OP:
			case _ERR_ASM_ILLEGAL_ARG:
				cout << "Illegal instruction" << endl;
				break;
			default:
				for (int i = 0; i < len; i++, add++)
					mem[add] = m_ret[i];
		}
	}
	return;
}

void unasm(string arg)
{
	static USHORT add = 0;
	if (arg != "")
		add = toNum(arg, 3);
	USHORT end = add + 0x40;
	string ins("");
	for (; add < end; )
	{
		printf("%04X:", add);
		add += unassembler(mem[add], mem[add + 1], mem[add + 2], ins);
		cout << ins << endl;
		if (mem[add] == 0)
			break;
	}
}

void dump(string arg)
{
	static UINT add = 0;
	if (arg != "")
		add = toNum(arg, 3);
	int i, j;
	for (i = 0; i < 8; i++)
	{
		printf("%04X:", add);
		for (j = 0; j < 8; j++)
		{
			printf("%04X ", mem[add]);
			add++;
			if (add > 0x10000)
			{
				add = 0;
				return;
			}
		}
		cout << endl;
	}
	return;
}

void enter(int argc, string args[])
{
	if (argc < 1)
	{
		cout << "  ^ Error" << endl;
		return;
	}
	UINT add = toNum(args[0], 3);
	for (int i = 1; i < argc && add < 0x10000; i++)
		mem[add++] = toNum(args[i], 3);
	return;
}

void regist(string arg = "")
{
	if (arg == "")
	{
		printf("A=%04X\tB=%04X\tC=%04X\tX=%04X\tY=%04X\tZ=%04X\tI=%04X\tJ=%04X\t", reg[0], reg[1], reg[2], reg[3], reg[4], reg[5], reg[6], reg[7]);
		cout << endl;
		printf("PC=%04X\tSP=%04X\tEX=%04X\tIA=%04X\t", pc, sp, ex, ia);
		cout << endl;
	}
	else
	{
		USHORT regt = 0;
		int ret = retGRegNum(arg, regt);
		if (ret == _ERR_ASM_NOERR)
		{
			cout << ucase(arg) << "=";
			printf("%04X", reg[regt]);
			cout << endl << ':';
			scanf("%x", &reg[regt]);
		}
		else 
		{
			if (arg == "pc")
			{
				cout << ucase(arg) << "=";
				printf("%04X", pc);
				cout << endl << ':';
				scanf("%x", &pc);
			}
			else if (arg == "sp")
			{
				cout << ucase(arg) << "=";
				printf("%04X", sp);
				cout << endl << ':';
				scanf("%x", &sp);
			}
			else if (arg == "ex")
			{
				cout << ucase(arg) << "=";
				printf("%04X", ex);
				cout << endl << ':';
				scanf("%x", &ex);
			}
			else if (arg == "ia")
			{
				cout << ucase(arg) << "=";
				printf("%04X", ia);
				cout << endl << ':';
				scanf("%x", &ia);
			}
			else
				return;
			//cout << endl;
		}
		cin.get();
	}
}

void trace(string arg = "")
{
	bool haveArg = false;
	UINT endAt = pc;
	if (arg != "")
	{
		endAt = toNum(arg, 2);
		haveArg = true;
	}
	string ins;
	do
	{
		doCode();
		regist();
		unassembler(mem[pc], mem[pc + 1], mem[pc + 2], ins);
		cout << "Next:" << ins << endl;
	}
	while (haveArg && pc < endAt);
}

void proceed()
{
#if __cplusplus >= 201103L || _MSC_VER >= 1700
	thread em(doCodeThread);
	em.detach();
	clockStarter();
	getchar();
	doCodeB = false;
#endif
}

struct label
{
	std::string str;
	USHORT pos;
	label();
	label(std::string _str, USHORT _pos)
	{
		str = _str;
		pos = _pos;
	}
};
typedef list<label> stringList;

struct gapPoint
{
	USHORT pos, len;
	gapPoint();
	gapPoint(USHORT _pos, USHORT _len)
	{
		pos = _pos;
		len = _len;
	}
};
typedef list<gapPoint> gapList;

void generate(string path, string arg = "")
{
	ifstream file(path);
	if (!file.is_open())
	{
		cout << "  ^ Error" << endl;
		return;
	}
	char line[100];
	int lineCount = 0;
	string insline;
	stringList lblLst, pendLst;
	gapList gapLst;
	string lbl;
	int pendCount = 0;
	int markPos = string::npos;
	stringList::const_iterator lblBeg, lblItr, lblEnd;
	stringList::const_iterator pendItr, pendEnd;
	gapList::const_iterator gapItr, gapEnd;
	USHORT *m = new USHORT[65536];
	USHORT add = toNum(arg);
	int len = 0, i;
	while (!file.eof())
	{
		lineCount++;
		file.getline(line, 100, '\n');
		insline = line;
		if (insline.length() < 1)
			continue;
		markPos = insline.find(':');
		if (markPos != string::npos)
		{
			if (markPos == 0)
			{
				lbl = insline.substr(1);
				insline = "";
				lblLst.push_back(label(lbl, add));
			}
			else
			{
				lbl = insline.substr(0, markPos);
				insline.erase(0, markPos + 1);
				lblLst.push_back(label(lbl, add));
			}
		}
		if (insline.length() < 1)
			continue;
		len = assembler(insline, m_ret, INS_RET_LEN);
		switch (len)
		{
			case _ERR_ASM_NOT_SUPPORTED:
				cout << "Instruction in line " << lineCount << " is not supported" << endl;
				goto _g_end;
			case _ERR_ASM_ILLEGAL:
			case _ERR_ASM_ILLEGAL_OP:
				cout << "Illegal instruction in line " << lineCount << endl;
				goto _g_end;
			case _ERR_ASM_ILLEGAL_ARG:
				pendLst.push_back(label(insline, add));
				add += 3;
				pendCount++;
				break;
			default:
				for (i = 0; i < len; i++, add++)
					m[add] = m_ret[i];
		}
	}
	pendEnd = pendLst.cend();
	lblBeg = lblLst.cbegin();
	lblEnd = lblLst.cend();
	int insLenAll = add;
	for (pendItr = pendLst.cbegin(); pendItr != pendEnd; pendItr++)
	{
		insline = pendItr->str;
		for (lblItr = lblBeg; lblItr != lblEnd; lblItr++)
		{
			markPos = insline.find(lblItr->str);
			if (markPos != string::npos)
				insline = insline.substr(0, markPos) + "0x" + toHEX(lblItr->pos) + insline.substr(markPos + lblItr->str.length());
			markPos = insline.find(lblItr->str);
			if (markPos != string::npos)
				insline = insline.substr(0, markPos) + "0x" + toHEX(lblItr->pos) + insline.substr(markPos + lblItr->str.length());
		}
		len = assembler(insline, m_ret, INS_RET_LEN);
		add = pendItr->pos;
		switch (len)
		{
			case _ERR_ASM_NOT_SUPPORTED:
				cout << "Instruction " << pendItr->str << " is not supported" << endl;
				goto _g_end;
			case _ERR_ASM_ILLEGAL:
			case _ERR_ASM_ILLEGAL_OP:
			case _ERR_ASM_ILLEGAL_ARG:
				cout << "Illegal instruction " << pendItr->str << endl;
				goto _g_end;
			default:
				for (i = 0; i < len; i++, add++)
					m[add] = m_ret[i];
				gapLst.push_back(gapPoint(add + len - 1, 3 - len));
		}
	}
	gapItr = gapLst.cbegin();
	gapEnd = gapLst.cend();
	int shift = 0;
	for (i = 0; i + shift < insLenAll; i++)
	{
		if (gapItr->pos == i)
		{
			shift += gapItr->len;
			gapItr++;
		}
		mem[i] = m[i + shift];
	}
	delete[] m;
	_g_end:file.close();
}

void printUsage()
{
	cout << "assemble\tA [address]" << endl;		//
	cout << "dump\t\tD [address]" << endl;			//
	cout << "enter\t\tE address [list]" << endl;	//
	cout << "generate [address]" << endl;						//
	cout << "load\t\tL address" << endl;			//
	cout << "name\t\tN path" << endl;				//
	cout << "proceed\t\tP" << endl;					//
	cout << "quit\t\tQ" << endl;					//
	cout << "register\tR [register]" << endl;		//
	cout << "trace\t\tT [end]" << endl;				//
	cout << "unassemble\tU [address]" << endl;		//
	cout << "write\t\tW start end" << endl;			//
	return;
}

int mainLoop()
{
	char _cmd[100];
	string cmd;
	string filePath;
	fstream file;
	char fileDatO = 0;
	USHORT fileDat = 0;
	int filePtr = 0, fileEndPtr = 0;
	int i, argn;
	cout << '-';
	while (true)
	{
		cin.getline(_cmd, 100, '\n');
		cmd = _cmd;
		cmd = trim(cmd);
		if (cmd.length() < 1)
		{
			cout << "  ^ Error" << endl << "-";
			continue;
		}
		if (cmd[0] != 'q')
		{
			argn = 0;
			m_arg[0] = "";
			int pos = cmd.find(' ');
			if (pos != string::npos)
			{
				argn++;
				m_arg[0] = ltrim(cmd.substr(pos + 1));
				cmd.erase(pos);
				for (i = 1; i < INS_RET_LEN; i++)
				{
					m_arg[i] = "";
					pos = m_arg[i - 1].find(' ');
					if (pos == string::npos)
						break;
					argn++;
					m_arg[i] = ltrim(m_arg[i - 1].substr(pos + 1));
					m_arg[i - 1].erase(pos);
				}
			}
		}
		if (cmd.length() != 1)
		{
			cout << "  ^ Error" << endl << "-";
			continue;
		}
		switch (cmd[0])
		{
			case 'q':
				return 0;
			case '?':
				printUsage();
				break;
			case 'd':
				dump(m_arg[0]);
				break;
			case 'e':
				enter(argn, m_arg);
				break;
			case 'r':
				regist(m_arg[0]);
				break;
			case 'a':
				assm(m_arg[0]);
				break;
			case 'u':
				unasm(m_arg[0]);
				break;
			case 't':
				trace(m_arg[0]);
				break;
			case 'p':
				proceed();
				break;
			case 'n':
				filePath = m_arg[0];
				break;
			case 'l':
				if (file.is_open())
					file.close();
				file.open(filePath, ios::in | ios::binary);
				filePtr = (USHORT)(toNum(m_arg[0]));
				if (!file.is_open())
				{
					cout << "  ^ Error:File not exists" << endl;
					break;
				}
				while (!file.eof())
				{
					file.get(fileDatO);
					fileDat = (BYTE)(fileDatO);
					file.get(fileDatO);
					fileDat = (fileDat << 8) + (BYTE)(fileDatO);
					mem[filePtr++] = fileDat;
				}
				file.close();
				break;
			case 'w':
				if (file.is_open())
					file.close();
				file.open(filePath, ios::out | ios::binary | ios::trunc);
				filePtr = (USHORT)(toNum(m_arg[0]));
				fileEndPtr = (USHORT)(toNum(m_arg[1]));
				for (; filePtr <= fileEndPtr; filePtr++)
				{
					file.put((mem[filePtr] & 0xFF00) >> 8);
					file.put(mem[filePtr] & 0xFF);
				}
				file.close();
				break;
			case 'g':
				generate(filePath, m_arg[0]);
				break;
			default:
				cout << "  ^ Error" << endl;
				break;
		}
		cout << '-';
	}
	return 0;
}

ifstream fin;
ofstream logout("log.log");

#define FUNC_COUNT 3
char *funcName[FUNC_COUNT] = {
	"getHWCount",
	"getInfo",
	"setHandle"
};
FARPROC funcAdd[FUNC_COUNT];

int main(int argc, char* argv[], char* envp[])
{
	fin.open("plugins.txt");
	if (!fin.is_open())
	{
		ofstream crtfile("plugins.txt");
		crtfile.close();
		fin.open("plugins.txt");
	}
	string filename("");
	char *fname = NULL;
	int len = 0, i;
	while (!fin.eof())
	{
		fin >> filename;
#ifdef _P_WIN
		len = filename.length();
		if (len < 1)
			continue;
		fname = new char[len + 1];
		for (i = 0; i < len; i++)
			fname[i] = filename[i];
		fname[len] = '\0';
		HMODULE plugin = LoadLibrary(filename.c_str());
		if (plugin != NULL)
		{
			logout << "Loaded plugin " << filename << endl;
			FARPROC hd = GetProcAddress(plugin, "init");
			if (hd != NULL)
			{
				int initRes = ((fInit)(*hd))();
				if (initRes != 0)
				{
					logout << "Failed to initialize plugin " << filename << " with error code 0x" << toHEX(initRes) << endl;
					FreeLibrary(plugin);
					delete fname;
					continue;
				}
			}
			bool loadSuccess = true;
			for (i = 0; i < FUNC_COUNT; i++)
			{
				hd = GetProcAddress(plugin, funcName[i]);
				if (hd == NULL)
				{
					logout << "Failed to find function entry point " << funcName[i] << " in plugin " << filename << endl;
					FreeLibrary(plugin);
					loadSuccess = false;
					break;
				}
				funcAdd[i] = hd;
			}
			if (loadSuccess)
			{
				int hwCount = ((fGetHWCount)(*funcAdd[0]))();
				hd = funcAdd[1];
				for (i = 0; i < hwCount; i++)
					hwt[hwn++] = ((fGetInfo)(*hd))(i);
				((fSetHandle)(*funcAdd[2]))(&setMem, &getMem, &setReg, &getReg, &additr);
			}
		}
		else
			logout << "Failed to load plugin " << filename << endl;
		delete fname;
#endif
#ifdef _P_LIN

#endif
	}
	return mainLoop();
}

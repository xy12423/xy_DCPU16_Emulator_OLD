#pragma once

#ifndef _H_ASM_FU
#define _H_ASM_FU

#include "define.h"
#include <limits.h>

std::string ltrim(std::string str)
{
	int len = str.length();
	int i;
	for (i = 0; i < len;i++)
		if (str[i] != ' ')
			break;
	return str.erase(0, i);
}

std::string rtrim(std::string str)
{
	int len = str.length();
	int i;
	for (i = len - 1; i > -1; i--)
		if (str[i] != ' ')
			break;
	if (i < len - 1)
		return str.erase(i + 1);
	return str;
}

std::string trim(std::string str)
{
	return ltrim(rtrim(str));
}

std::string lcase(std::string str)
{
	int len = str.length();
	for (int i = 0; i < len; i++)
		str[i] = tolower(str[i]);
	return str;
}

std::string ucase(std::string str)
{
	int len = str.length();
	for (int i = 0; i < len; i++)
		str[i] = toupper(str[i]);
	return str;
}

int numLevel[] = { 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};

bool canBeNum(std::string str)
{
	int len = str.length(), level = -1;
	if (len == 0)
		return false;
	if (str[0] == '0' && len > 1)
	{
		if (len > 2 && str[1] == 'x')
		{
			level = 2;
			str.erase(0, 2);
			len -= 2;
		}
		else
		{
			level = 0;
			str.erase(0, 1);
			len--;
		}
	}
	else if (str[len - 1] == 'h')
	{
		level = 2;
		str.erase(len - 1, 1);
		len--;
	}
	else
		level = 1;
	for (int i = 0; i < len; i++)
		if (numLevel[(BYTE)(str[i])] > level)
			return false;
	return true;
}

int toNum(std::string str, int type = 3)
{
	int ret = INT_MIN, len = str.length();
	std::stringstream ss;
	if (str[len - 1] == 'h')
	{
		str.erase(len - 1, 1);
		ss << std::hex << str;
		ss >> ret;
		//sscanf(str.c_str(), "%x", &ret);
	}
	else
	{
		switch (type)
		{
			case 3:
				//sscanf(str.c_str(), "%i", &ret);
				if (len == 0)
					return false;
				if (str[0] == '0' && len > 1)
				{
					if (len > 2 && str[1] == 'x')
					{
						str.erase(0, 2);
						ss << std::hex << str;
						ss >> ret;
					}
					else
					{
						str.erase(0, 1);
						ss << std::oct << str;
						ss >> ret;
					}
				}
				else if (str[len - 1] == 'h')
				{
					str.erase(len - 1, 1);
					ss << std::hex << str;
					ss >> ret;
				}
				else
				{
					ss << std::dec << str;
					ss >> ret;
				}
				break;
			case 2:
				//sscanf(str.c_str(), "%x", &ret);
				ss << std::hex << str;
				ss >> ret;
				break;
			case 1:
				//sscanf(str.c_str(), "%d", &ret);
				ss << std::dec << str;
				ss >> ret;
				break;
			case 0:
				//sscanf(str.c_str(), "%o", &ret);
				ss << std::oct << str;
				ss >> ret;
				break;
		}
	}
	return ret;
}

std::string toHEX(unsigned int n)
{
	std::stringstream ss;
	std::string ret;
	ss << std::hex << n;
	ss >> ret;
	if (ret.length() < 4)
		ret = std::string(4 - ret.length(), '0') + ucase(ret);
	return ret;
}

#endif
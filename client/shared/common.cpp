#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "config.h"

std::vector<std::string>
split(const std::string &str, const char delim, std::vector<std::string> &elems)
{
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string>
split(const std::string &s, const char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

std::wstring
widen(const char *str)
{
	std::wstring output;
	wchar_t *wcs;
	int sz;

	sz = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	if (sz == 0xFFFD) return output;
	wcs = new wchar_t[++sz];
	MultiByteToWideChar(CP_UTF8, 0, str, -1, wcs, sz);
	output = wcs;
	return output;
}

std::wstring
widen(const std::string str)
{
	std::wstring output;
	wchar_t *wcs;
	int sz;

	sz = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	if (sz == 0xFFFD) return output;
	wcs = new wchar_t[++sz];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wcs, sz);
	output = wcs;
	return output;
}

std::string
narrow(const wchar_t *str)
{
	std::string output;
	char *mbstr;
	int sz;

	sz = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	if (sz == 0xFFFD)
		return output;
	mbstr = new char[++sz];
	WideCharToMultiByte(CP_UTF8, 0, str, -1, mbstr, sz, NULL, NULL);
	output = mbstr;
	return output;
}

std::string
narrow(const std::wstring str)
{
	std::string output;
	char *mbstr;
	int sz;

	sz = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
	if (sz == 0xFFFD)
		return output;
	mbstr = new char[++sz];
	WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, mbstr, sz, NULL, NULL);
	output = mbstr;
	return output;
}

int docloud_is_correct_filetype(const char *name)
{
	std::string str;
	std::vector<std::string> list;

	list = split(config::getStr("accepted_filetypes"), ';');

	str = name;
	str.erase(0, str.find_last_of('.'));

	std::vector<std::string>::iterator it;
	for (it = list.begin(); it != list.end(); it ++) {
		if (str == (*it)) {
			return 1;
		}
	}

	return 0;
}

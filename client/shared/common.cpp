#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "config.h"

std::vector<std::wstring>
splitw(const std::wstring &str, wchar_t delim, std::vector<std::wstring> &elems)
{
	std::wstringstream ss(str);
	std::wstring item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::wstring>
splitw(const std::wstring &s, wchar_t delim)
{
    std::vector<std::wstring> elems;
    splitw(s, delim, elems);
    return elems;
}

int docloud_is_correct_filetype(const wchar_t *name)
{
	std::wstring str;
	std::vector<std::wstring> list;

	list = splitw(config::getStr(L"accepted_filetypes"), L';');

	str = name;
	str.erase(0, str.find_last_of('.'));

	std::vector<std::wstring>::iterator it;
	for (it = list.begin(); it != list.end(); it ++) {
		if (str == (*it)) {
			return 1;
		}
	}

	return 0;
}

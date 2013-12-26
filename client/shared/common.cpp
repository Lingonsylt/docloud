#include <string>
#include <vector>
#include <sstream>
#include <iostream>

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

int docloud_is_correct_filetype(wchar_t *name)
{
	wchar_t *ptr;

	return 1;
}

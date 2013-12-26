#ifndef COMMON_H
#define COMMON_H
#include <vector>
#include <string>

std::vector<std::wstring> splitw(const std::wstring &str, wchar_t delim, std::vector<std::wstring> &elems);
std::vector<std::wstring> splitw(const std::wstring &s, wchar_t delim);

int docloud_is_correct_filetype(const wchar_t *name);

#endif

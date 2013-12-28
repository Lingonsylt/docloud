#ifndef COMMON_H
#define COMMON_H
#include <vector>
#include <string>

std::vector<std::string> split(const std::string &str, const char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, const char delim);

std::wstring widen(const char *str);
std::wstring widen(const std::string str);
std::string narrow(const wchar_t *str);
std::string narrow(const std::wstring str);

int docloud_is_correct_filetype(const char *name);

#endif

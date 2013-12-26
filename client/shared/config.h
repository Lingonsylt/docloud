#ifndef CONFIG_H
#define CONFIG_H

namespace config {
	std::wstring getStr(const wchar_t *config);
	std::vector<std::wstring> getList(const wchar_t *config);
}


#endif

#ifndef CONFIG_H
#define CONFIG_H

namespace config {
	bool setStr(const wchar_t *config, const wchar_t *value);
	bool setStr(const wchar_t *config, std::wstring value);

	std::wstring getStr(const wchar_t *config);
	std::vector<std::wstring> getList(const wchar_t *config);
}


#endif

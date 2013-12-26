#include <string>
#include "common.h"
#include "reg.h"

namespace config {
	int getInt(wchar_t *config) {

	}

	std::wstring getStr(const wchar_t *config) {
		std::wstring result;
		wchar_t *str;
			
		str = RegGetKeyString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\docloud\\docloud", config);
		if (str != NULL)
			result = str;
		return result;
	}

	std::vector<std::wstring> getList(const wchar_t *config) {
		std::vector<std::wstring> list;
		std::wstring str;

		str = getStr(config);

		list = splitw(str, L'\n');
		return list;
	}
}

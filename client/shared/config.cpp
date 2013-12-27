#include <string>
#include "common.h"
#include "reg.h"

namespace config {
	int getInt(wchar_t *config) {

	}

	bool setStr(const wchar_t *config, const wchar_t *value) {
		std::wstring result;
		HRESULT ret;
			
		ret = RegSetKeyString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\docloud\\docloud", config, value);
		if (!SUCCEEDED(ret))
			return false;
		return true;
	}

	bool setStr(const wchar_t *config, std::wstring value) {

		std::wstring result;
		HRESULT ret;
			
		return setStr(config, value.c_str());
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

#include <string>
#include "common.h"
#include "reg.h"

namespace config {
	int getInt(const char *config) {

	}

	bool setStr(const char *config, const char *value) {
		HRESULT ret;
			
		ret = RegSetKeyString(HKEY_LOCAL_MACHINE, "SOFTWARE\\docloud\\docloud", config, value);
		if (!SUCCEEDED(ret))
			return false;
		return true;
	}

	bool setStr(const char *config, std::string value) {

		HRESULT ret;
			
		return setStr(config, value.c_str());
	}

	std::string getStr(const char *config) {
		std::string result;
		char *str;
			
		str = RegGetKeyString(HKEY_LOCAL_MACHINE, "SOFTWARE\\docloud\\docloud", config);
		if (str != NULL)
			result = str;
		return result;
	}

	std::vector<std::string> getList(const char *config) {
		std::vector<std::string> list;
		std::string str;

		str = getStr(config);

		list = split(str, ';');
		return list;
	}
}

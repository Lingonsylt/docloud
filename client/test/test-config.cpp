#include <string>
#include <vector>
#include "config.h"
#include "test.h"
int main(int argc, char *argv[])
{
	std::string str;

	if (config::setStr("test_config_str", "abcdef12345") == false) {
		TEST_ERR("config::setStr(const char *)");
	} else {
		TEST_PASS("config::setStr(const char *)");
	}

	str = "abcdef12345";
	if (config::setStr("test_config_str", str) == false) {
		TEST_ERR("config::setStr(std::string)");
	} else {
		TEST_PASS("config::setStr(std::string)");
	}

	str = config::getStr("test_config_str");
	if (str != "abcdef12345") {
		TEST_ERR("config::getStr()");
	} else {
		TEST_PASS("config::getStr()");
	}

	if (config::setStr("test_config_str", "abc;def;geh;ijk;lmn;opq") == false) {
		TEST_ERR("config::setStr(list)");
	} else {
		TEST_PASS("config::setStr(list)");
	}

	std::vector<std::string> list;
	list = config::getList("test_config_str");
	if (list.size() != 6 ||
	    list[0] != "abc" ||
	    list[1] != "def" ||
	    list[2] != "geh" ||
	    list[3] != "ijk" ||
	    list[4] != "lmn" ||
	    list[5] != "opq") {
		TEST_ERR("config::getList()");
	} else {
		TEST_PASS("config::getList()");
	}

	return 0;
}

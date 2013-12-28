#ifndef CONFIG_H
#define CONFIG_H

namespace config {
	bool setStr(const char *config, const char *value);
	bool setStr(const char *config, std::string value);

	std::string getStr(const char *config);
	std::vector<std::string> getList(const char *config);
}


#endif

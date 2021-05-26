#include <unordered_set>
#include "Plugin.h"

class KPluginManager {
private:
	std::unordered_set<KPlugin*> plugins;
public:
	KPluginManager(const std::string plugins_path);
	void onJoin(const std::string &uuid);
};

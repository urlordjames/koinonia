#include "PluginManager.h"
#include <filesystem>

KPluginManager::KPluginManager(const std::string plugin_path) {
	std::filesystem::create_directories(plugin_path);
	std::filesystem::directory_iterator plugin_iterator(plugin_path);

	for (auto script_path : plugin_iterator) {
		KPlugin *plugin = new KPlugin(script_path.path());
		plugins.insert(plugin);
	}
}

void KPluginManager::onJoin() {
	for (auto p : plugins) {
		p->onJoin();
	}
}

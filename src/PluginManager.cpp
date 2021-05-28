#include "PluginManager.h"
#include <filesystem>

KPluginManager::KPluginManager(const std::string plugin_path) {
	std::filesystem::create_directories(plugin_path);
	std::filesystem::directory_iterator plugin_iterator(plugin_path);

	// if JSON parse cannot cast to int it will return 0
	// ids will start at 1 so malfunctioning plugins cannot pass messages to other plugins
	int id = 1;

	for (auto script_path : plugin_iterator) {
		KPlugin *plugin = new KPlugin(script_path.path(), id++);
		plugins.insert(plugin);
	}
}

void KPluginManager::onJoin(const std::string &uuid) {
	for (auto p : plugins) {
		p->onJoin(uuid);
	}
}

void KPluginManager::passMsg(int plugin_id, const std::string &msg) {
	for (auto p : plugins) {
		int id = p->getId();
		if (plugin_id == id) {
			p->onMsg(msg);
			return;
		}
	}
}

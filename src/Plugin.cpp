#include "Plugin.h"
#include "StreamSock.h"
#include <iostream>

int print_lua(lua_State *L) {
	std::cout << luaL_checkstring(L, 1) << std::endl;
	return 0;
}

KPlugin::KPlugin(const std::string plugin_path) {
	L = luaL_newstate();

	lua_register(L, "print", print_lua);

	int result = luaL_dofile(L, plugin_path.c_str());

	if (result != LUA_OK) {
		std::cerr << "ERROR LOADING PLUGIN:\n" << lua_tostring(L, -1) << std::endl;
		exit(EXIT_FAILURE);
	}
}

void KPlugin::onJoin(const std::string &uuid) {
	lua_getglobal(L, "on_join");
	lua_pushstring(L, uuid.c_str());
	int result = lua_pcall(L, 1, 0, 0);

	if (result != LUA_OK) {
		std::cerr << lua_tostring(L, -1) << std::endl;
	}
}

KPlugin::~KPlugin() {
	lua_close(L);
}

#include "Plugin.h"
#include "Messages.h"
#include "StreamSock.h"
#include "SocketInfo.h"
#include <iostream>
#include <optional>

int print_lua(lua_State *L) {
	std::cout << luaL_checkstring(L, 1) << std::endl;
	return 0;
}

std::optional<WebSocketConnectionPtr> getParticipant(const std::string &uuid) {
	for (auto i : StreamSock::participants) {
		auto p = i->getContext<SocketInfo>();
		if (p->getUuid() == uuid) {
			return std::optional<WebSocketConnectionPtr>(i);
		}
	}

	return std::optional<WebSocketConnectionPtr>(nullptr);
}

int send_msg(lua_State *L) {
	std::string uuid(luaL_checkstring(L, 1));
	std::optional<WebSocketConnectionPtr> result = getParticipant(uuid);
	if (result.has_value()) {
		WebSocketConnectionPtr wsConnPtr = result.value();
		Json::Value msg;
		msg["message"] = luaL_checkstring(L, 2);
		wsConnPtr->send(pluginMsg(msg));
	} else {
		std::cerr << "ERROR: no such uuid in send_msg from plugin" << std::endl;
	}
	return 0;
}

KPlugin::KPlugin(const std::string plugin_path) {
	L = luaL_newstate();

	lua_register(L, "print", print_lua);
	lua_register(L, "send_msg", send_msg);

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

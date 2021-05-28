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
		msg["message"] = luaL_checkstring(L, 3);
		wsConnPtr->send(pluginMsg(luaL_checkstring(L, 2), msg));
	} else {
		std::cerr << "ERROR: no such uuid in send_msg from plugin" << std::endl;
	}
	return 0;
}

int send_module(lua_State *L) {
	std::string uuid(luaL_checkstring(L, 1));
	std::string script(luaL_checkstring(L, 2));
	std::optional<WebSocketConnectionPtr> result = getParticipant(uuid);
	if (result.has_value()) {
		WebSocketConnectionPtr wsConnPtr = result.value();
		wsConnPtr->send(moduleMsg(script));
	} else {
		std::cerr << "ERROR: no such uuid in send_module from plugin" << std::endl;
	}
	return 0;
}

int get_id(lua_State *L) {
	lua_pushinteger(L, luaL_checkinteger(L, lua_upvalueindex(1)));
	return 1;
}

KPlugin::KPlugin(const std::string plugin_path, int id) {
	this->id = id;
	L = luaL_newstate();

	luaL_requiref(L, "string", luaopen_string, true);

	lua_register(L, "print", print_lua);
	lua_register(L, "send_msg", send_msg);
	lua_register(L, "send_module", send_module);

	lua_pushinteger(L, id);
	lua_pushcclosure(L, get_id, 1);
	lua_setglobal(L, "get_id");

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

void KPlugin::onMsg(const std::string &msg) {
	lua_getglobal(L, "on_msg");
	lua_pushstring(L, msg.c_str());
	int result = lua_pcall(L, 1, 0, 0);

	if (result != LUA_OK) {
		std::cerr << lua_tostring(L, -1) << std::endl;
	}
}

int KPlugin::getId() {
	return id;
}

KPlugin::~KPlugin() {
	lua_close(L);
}

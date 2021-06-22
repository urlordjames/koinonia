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
	int id = luaL_checkinteger(L, lua_upvalueindex(1));
	std::string uuid(luaL_checkstring(L, 1));

	std::optional<WebSocketConnectionPtr> result = getParticipant(uuid);
	if (result.has_value()) {
		WebSocketConnectionPtr wsConnPtr = result.value();
		wsConnPtr->send(pluginMsg(id, luaL_checkstring(L, 2)));
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

#if LUA_VERSION_NUM >= 502
	luaL_requiref(L, "string", luaopen_string, true);
#else
	lua_pushcfunction(L, luaopen_string);
	lua_call(L, 0, 0);
#endif

	lua_register(L, "print", print_lua);
	lua_register(L, "send_module", send_module);

	lua_pushinteger(L, id);
	lua_pushcclosure(L, send_msg, 1);
	lua_setglobal(L, "send_msg");

	lua_pushinteger(L, id);
	lua_pushcclosure(L, get_id, 1);
	lua_setglobal(L, "get_id");

	int result = luaL_dofile(L, plugin_path.c_str());

	if (result != LUA_OK) {
		std::cerr << "ERROR LOADING PLUGIN:\n" << lua_tostring(L, -1) << std::endl;
		exit(EXIT_FAILURE);
	}
}

bool KPlugin::call_func(const char *fname, const std::string &arg) {
	// TODO: workaround for lua >= 5.2
	if (lua_getglobal(L, fname) != LUA_TFUNCTION) {
		return true;
	}

	lua_pushstring(L, arg.c_str());
	int result = lua_pcall(L, 1, 0, 0);

	if (result != LUA_OK) {
		std::cerr << lua_tostring(L, -1) << std::endl;
		return false;
	}

	return true;
}

void KPlugin::onJoin(const std::string &uuid) {
	call_func("on_join", uuid);
}

void KPlugin::onLeave(const std::string &uuid) {
	call_func("on_leave", uuid);
}

void KPlugin::onMsg(const std::string &msg) {
	call_func("on_msg", msg);
}

int KPlugin::getId() {
	return id;
}

KPlugin::~KPlugin() {
	lua_close(L);
}

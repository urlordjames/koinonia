#include <string>

extern "C" {
	#include <lauxlib.h>
	#include <lualib.h>
}

// lua 5.1 compatibility
#ifndef LUA_OK
#define LUA_OK 0
#endif

// short for koinonia plugin
class KPlugin {
private:
	lua_State *L;
	int id;
public:
	KPlugin(const std::string script_path, int id);
	void onJoin(const std::string &uuid);
	void onLeave(const std::string &uuid);
	void onMsg(const std::string &msg);
	int getId();
	~KPlugin();
};

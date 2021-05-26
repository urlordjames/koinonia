#include <string>

extern "C" {
	#include <lauxlib.h>
	#include <lualib.h>
}

// short for koinonia plugin
class KPlugin {
private:
	lua_State *L;
public:
	KPlugin(const std::string script_path);
	void onJoin();
	~KPlugin();
};

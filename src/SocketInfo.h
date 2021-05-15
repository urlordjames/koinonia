#include <string>
#include <mutex>

class SocketInfo {
private:
	std::string uuid = "";
	std::mutex uuid_mutex;
public:
	const std::string getUuid();
	void setUuid(const std::string &uuid);
};

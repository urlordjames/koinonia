#include <string>
#include <mutex>

class SocketInfo {
private:
	std::string uuid;
	std::mutex uuid_mutex;
	int room;
public:
	SocketInfo(const std::string &uuid, int room);
	const std::string getUuid();
	int getRoom();
};

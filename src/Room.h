#include <drogon/WebSocketController.h>
#include <optional>

class Room {
private:
	std::unordered_map<std::string, drogon::WebSocketConnectionPtr> participants;
	std::mutex mutex;
public:
	std::optional<drogon::WebSocketConnectionPtr> getParticipant(const std::string &uuid);
	std::unordered_set<drogon::WebSocketConnectionPtr> allParticipants();
	void join(std::string uuid, drogon::WebSocketConnectionPtr wsConnPtr);
	void leave(const std::string &uuid);
};

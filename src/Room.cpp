#include "Room.h"

std::optional<drogon::WebSocketConnectionPtr> Room::getParticipant(const std::string &uuid) {
	std::lock_guard<std::mutex> lock(mutex);

	try {
		return std::make_optional<drogon::WebSocketConnectionPtr>(participants.at(uuid));
	} catch (std::out_of_range) {
		//return std::make_optional<drogon::WebSocketConnectionPtr>(std::nullopt);
		return std::nullopt;
	};
}

std::unordered_set<drogon::WebSocketConnectionPtr> Room::allParticipants() {
	std::lock_guard<std::mutex> lock(mutex);

	std::unordered_set<drogon::WebSocketConnectionPtr> set;

	for (auto participant : participants) {
		set.insert(participant.second);
	}

	return set;
}

void Room::join(std::string uuid, drogon::WebSocketConnectionPtr wsConnPtr) {
	std::lock_guard<std::mutex> lock(mutex);

	participants.insert({uuid, wsConnPtr});
}

void Room::leave(const std::string &uuid) {
	std::lock_guard<std::mutex> lock(mutex);

	participants.erase(uuid);
}

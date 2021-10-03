#include "Room.h"
#include "Messages.h"

std::optional<drogon::WebSocketConnectionPtr> Room::getParticipant(const std::string &uuid) {
	std::lock_guard<std::mutex> lock(mutex);

	try {
		return std::make_optional<drogon::WebSocketConnectionPtr>(participants.at(uuid));
	} catch (std::out_of_range) {
		return std::nullopt;
	};
}

std::unordered_set<drogon::WebSocketConnectionPtr> Room::allParticipants() {
	std::unordered_set<drogon::WebSocketConnectionPtr> set;

	std::lock_guard<std::mutex> lock(mutex);

	for (auto participant : participants) {
		set.insert(participant.second);
	}

	return set;
}

void Room::join(std::string uuid, drogon::WebSocketConnectionPtr wsConnPtr) {
	const std::string join_msg = joinMsg(uuid);
	Json::Value to_send(Json::arrayValue);

	std::lock_guard<std::mutex> lock(mutex);

	for (auto participant : participants) {
		// inform all participants of new participant
		participant.second->send(join_msg);

		// sync state with new connection
		// this is a json object because I may want to attach additional information in the future like a username
		Json::Value part_json;
		part_json["uuid"] = participant.first;
		to_send.append(part_json);
	}

	participants.insert({uuid, wsConnPtr});

	wsConnPtr->send(syncMsg(to_send));
}

void Room::leave(const std::string &uuid) {
	const std::string leave_msg = leaveMsg(uuid);

	std::lock_guard<std::mutex> lock(mutex);

	participants.erase(uuid);

	for (auto participant : participants) {
		participant.second->send(leave_msg);
	}
}

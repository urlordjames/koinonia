#include "SocketInfo.h"

SocketInfo::SocketInfo(const std::string &uuid, int room) {
	this->uuid = uuid;
	this->room = room;
}

const std::string SocketInfo::getUuid() {
	std::lock_guard<std::mutex> lock(uuid_mutex);
	return uuid;
}

int SocketInfo::getRoom() {
	return this->room;
}

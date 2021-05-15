#include "SocketInfo.h"

const std::string SocketInfo::getUuid() {
	std::lock_guard<std::mutex> lock(uuid_mutex);
	return uuid;
}

void SocketInfo::setUuid(const std::string &uuid) {
	std::lock_guard<std::mutex> lock(uuid_mutex);
	this->uuid = uuid;
}

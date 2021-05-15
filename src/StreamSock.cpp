#include "StreamSock.h"
#include "SocketInfo.h"
#include "Util.h"
#include "Messages.h"

enum class msgType {
	uuid,
	offer,
	answer,
	ice,
};

const std::unordered_map<std::string, msgType> typelookup = {
	{"uuid", msgType::uuid},
	{"offer", msgType::offer},
	{"answer", msgType::answer},
	{"ice", msgType::ice},
};

std::unordered_set<WebSocketConnectionPtr> participants;
std::mutex participants_mutex;

void StreamSock::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string&& message, const WebSocketMessageType& type) {
	if (type != WebSocketMessageType::Text) return;

	Json::Value m;
	try {
		m = parseJSON(&message);
	} catch (std::string err) {
		wsConnPtr->send(errorMsg(err));
		return;
	}

	std::string typestring;
	try {
		typestring = m["type"].asString();
	} catch (Json::Exception err) {
		wsConnPtr->send(errorMsg(err.what()));
		return;
	}

	msgType msgtype;
	try {
		msgtype = typelookup.at(typestring);
	} catch (std::out_of_range err) {
		wsConnPtr->send(errorMsg("no such message type: " + typestring));
		return;
	}

	auto info = wsConnPtr->getContext<SocketInfo>();
	switch (msgtype) {
		case msgType::uuid:
			wsConnPtr->send(uuidMsg(info->uuid));
			break;
		case msgType::offer:
			// TODO: maybe use std::map instead
			participants_mutex.lock();

			for (auto i : participants) {
				auto p = i->getContext<SocketInfo>();
				if (p->uuid == m["uuid"].asString()) {
					i->send(offerMsg(info->uuid, m["offer"]));
					participants_mutex.unlock();
					return;
				}
			}

			participants_mutex.unlock();

			wsConnPtr->send(errorMsg("no such uuid"));
			break;
		case msgType::answer:
			participants_mutex.lock();

			for (auto i : participants) {
				auto p = i->getContext<SocketInfo>();
				if (p->uuid == m["uuid"].asString()) {
					i->send(answerMsg(info->uuid, m["answer"]));
					participants_mutex.unlock();
					return;
				}
			}

			participants_mutex.unlock();

			wsConnPtr->send(errorMsg("no such uuid"));
			break;
		case msgType::ice:
			participants_mutex.lock();

			for (auto i : participants) {
				auto p = i->getContext<SocketInfo>();
				if (p->uuid == m["uuid"].asString()) {
					i->send(iceMsg(info->uuid, m["candidate"]));
					participants_mutex.unlock();
					return;
				}
			}

			participants_mutex.unlock();

			wsConnPtr->send(errorMsg("no such uuid"));
			break;
		default:
			wsConnPtr->send(debugMsg("no server implementation for message type: " + typestring));
	}
}

void StreamSock::handleNewConnection(const HttpRequestPtr &req,const WebSocketConnectionPtr& wsConnPtr) {
	auto info = std::make_shared<SocketInfo>();
	info->uuid = drogon::utils::getUuid();
	wsConnPtr->setContext(info);

	Json::Value to_send(Json::arrayValue);

	participants_mutex.lock();

	for (auto i : participants) {
		// inform all participants of new participant
		i->send(joinMsg(info->uuid));

		// sync state with new connection
		// this is a json object because I may want to attach additional information in the future like a username
		auto p = i->getContext<SocketInfo>();
		Json::Value part_json;
		part_json["uuid"] = p->uuid;
		to_send.append(part_json);
	}

	participants.insert(wsConnPtr);
	participants_mutex.unlock();

	wsConnPtr->send(syncMsg(to_send));
}

void StreamSock::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr) {
	std::string uuid = wsConnPtr->getContext<SocketInfo>()->uuid;

	participants_mutex.lock();
	participants.erase(wsConnPtr);

	for (auto i : participants) {
		i->send(leaveMsg(uuid));
	}

	participants_mutex.unlock();
}

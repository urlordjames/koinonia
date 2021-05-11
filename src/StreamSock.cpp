#include "StreamSock.h"
#include "SocketInfo.h"
#include "Util.h"

enum class msgType {
	uuid,
	sync,
	offer,
	answer,
	ice,
};

const std::unordered_map<std::string, msgType> typelookup = {
	{"uuid", msgType::uuid},
	{"sync", msgType::sync},
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
		case msgType::sync:
			{
				Json::Value to_send(Json::arrayValue);

				participants_mutex.lock();

				for (auto i : participants) {
					auto participant = i->getContext<SocketInfo>();
					if (i != wsConnPtr) {
						// this is a json object because I may want to attach additional information in the future like a username
						Json::Value part_json;
						part_json["uuid"] = participant->uuid;
						to_send.append(part_json);
					}
				}

				participants_mutex.unlock();

				wsConnPtr->send(syncMsg(to_send));
			}
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

	participants_mutex.lock();

	for (auto i : participants) {
		i->send(joinMsg(info->uuid));
	}

	participants.insert(wsConnPtr);
	participants_mutex.unlock();

	wsConnPtr->send(debugMsg("hello!"));
}

void StreamSock::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr) {
	participants_mutex.lock();
	std::string uuid = wsConnPtr->getContext<SocketInfo>()->uuid;

	participants.erase(wsConnPtr);

	for (auto i : participants) {
		i->send(leaveMsg(uuid));
	}

	participants_mutex.unlock();
}

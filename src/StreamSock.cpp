#include "StreamSock.h"
#include "SocketInfo.h"
#include "Util.h"

enum class msgType {
	join,
	sync,
	answer,
	debug
};

const std::unordered_map<std::string, msgType> typelookup = {
	{"join", msgType::join},
	{"sync", msgType::sync},
	{"answer", msgType::answer},
	{"debug", msgType::debug}
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
		case msgType::join:
			info->sdp = stringify(&m["sdp"]);

			participants_mutex.lock();
			participants.insert(wsConnPtr);
			participants_mutex.unlock();

			break;
		case msgType::sync:
			{
				Json::Value to_send(Json::arrayValue);

				participants_mutex.lock();

				for (auto i : participants) {
					auto participant = i->getContext<SocketInfo>();
					if (i != wsConnPtr) {
						Json::Value part_json;
						part_json["sdp"] = participant->sdp;
						part_json["uuid"] = participant->uuid;
						to_send.append(part_json);
					}
				}

				participants_mutex.unlock();

				wsConnPtr->send(syncMsg(&to_send));
			}
			break;
		case msgType::answer:
			// TODO: maybe use std::map instead
			participants_mutex.lock();

			for (auto i : participants) {
				auto p = i->getContext<SocketInfo>();
				if (p->uuid == m["uuid"].asString()) {
					i->send(answerMsg(&m["message"]));
					participants_mutex.unlock();
					return;
				}
			}

			participants_mutex.unlock();

			wsConnPtr->send(errorMsg("no such uuid"));
			break;
		case msgType::debug:
			wsConnPtr->send(debugMsg("your sdp is: " + info->sdp));
			break;
		default:
			wsConnPtr->send(debugMsg("no server implementation for message type: " + typestring));
	}
}

void StreamSock::handleNewConnection(const HttpRequestPtr &req,const WebSocketConnectionPtr& wsConnPtr) {
	auto info = std::make_shared<SocketInfo>();
	info->uuid = drogon::utils::getUuid();
	wsConnPtr->setContext(info);
	wsConnPtr->send(debugMsg("hello!"));
}

void StreamSock::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr) {
	participants_mutex.lock();
	participants.erase(wsConnPtr);
	participants_mutex.unlock();
}

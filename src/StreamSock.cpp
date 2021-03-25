#include "StreamSock.h"
#include "SocketInfo.h"
#include "Util.h"

enum types{
	join,
	sync,
	answer,
	debug
};

const std::unordered_map<std::string, types> typelookup = {
	{"join", join},
	{"sync", sync},
	{"answer", answer},
	{"debug", debug}
};

std::unordered_set<WebSocketConnectionPtr> participants;

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

	types msgtype;
	try {
		msgtype = typelookup.at(typestring);
	} catch (std::out_of_range err) {
		wsConnPtr->send(errorMsg("no such message type: " + typestring));
		return;
	}

	auto info = wsConnPtr->getContext<SocketInfo>();
	switch (msgtype) {
		case join:
			info->sdp = stringify(&m["sdp"]);
			participants.insert(wsConnPtr);
			break;
		case sync:
			{
				Json::Value to_send(Json::arrayValue);
				for (auto i = participants.begin(); i != participants.end(); i++) {
					auto participant = (*i)->getContext<SocketInfo>();
					if (*i != wsConnPtr) {
						Json::Value part_json;
						part_json["sdp"] = participant->sdp;
						part_json["uuid"] = participant->uuid;
						to_send.append(part_json);
					}
				}
				wsConnPtr->send(syncMsg(&to_send));
			}
			break;
		case answer:
			// optimize later (maybe using std::unordered_map)
			for (auto i = participants.begin(); i != participants.end(); i++) {
				auto p = (*i)->getContext<SocketInfo>();
				if (p->uuid == m["uuid"].asString()) {
					(*i)->send(debugMsg(m["message"].asString()));
					return;
				}
			}
			wsConnPtr->send(errorMsg("no such uuid"));
			break;
		case debug:
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
	participants.erase(wsConnPtr);
}

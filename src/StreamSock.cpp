#include "StreamSock.h"
#include "SocketInfo.h"
#include "Util.h"

enum types{
	join,
	sync,
	debug
};

const std::unordered_map<std::string, types> typelookup = {
	{"join", join},
	{"sync", sync},
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
						to_send.append(participant->sdp);
					}
				}
				wsConnPtr->send(debugMsg(stringify(&to_send)));
			}
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
	wsConnPtr->setContext(info);
	wsConnPtr->send(debugMsg("hello!"));
}

void StreamSock::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr) {
	participants.erase(wsConnPtr);
}

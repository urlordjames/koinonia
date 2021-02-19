#include "StreamSock.h"
#include "SocketInfo.h"
#include "Util.h"

enum types{
	sdp,
	stream,
	debug
};

const std::unordered_map<std::string, types> typelookup = {
	{"sdp", sdp},
	{"stream", stream},
	{"debug", debug}
};

void StreamSock::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string&& message, const WebSocketMessageType& type) {
	if (type != WebSocketMessageType::Text) return;
	try {
		std::unique_ptr<Json::Value> m = parseJSON(&message);
		std::string typestring = (*m)["type"].asString();
		try {
			const types type = typelookup.at(typestring);
			auto info = wsConnPtr->getContext<SocketInfo>();
			switch (type) {
				case sdp:
					info->sdp = (*m)["sdp"].asString();
					break;
				case debug:
					wsConnPtr->send(debugMsg("your sdp is: " + info->sdp));
					break;
				default:
					wsConnPtr->send(debugMsg("no server implementation for message type: " + typestring));
			}
		} catch (std::out_of_range err) {
			wsConnPtr->send(errorMsg("no such message type: " + typestring));
		}
	} catch (JSONCPP_STRING err) {
		wsConnPtr->send(errorMsg(err));
		return;
	}
}

void StreamSock::handleNewConnection(const HttpRequestPtr &req,const WebSocketConnectionPtr& wsConnPtr) {
	auto info = std::make_shared<SocketInfo>();
	wsConnPtr->setContext(info);
	wsConnPtr->send(debugMsg("hello!"));
}

void StreamSock::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr) {
	// stuff
}

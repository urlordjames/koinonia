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

WebSocketConnectionPtr host;
bool hasHost = false;

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
		case sdp:
			info->sdp = stringify(&m["sdp"]);
			break;
		case stream:
			if (hasHost) {
				wsConnPtr->send(debugMsg("already host"));
			}
			else {
				host = wsConnPtr;
				info->host = true;
				hasHost = true;
				wsConnPtr->send(debugMsg("OK"));
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
	auto info = wsConnPtr->getContext<SocketInfo>();
	if (info->host) {
		hasHost = false;
		host = NULL;
	}
}

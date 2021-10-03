#include "StreamSock.h"
#include "Util.h"
#include "Messages.h"

std::unordered_map<int, Room> StreamSock::rooms;

#ifdef USE_LUA_PLUGINS
#include "PluginManager.h"
KPluginManager pluginManager("plugins");
#endif

void StreamSock::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string&& message, const WebSocketMessageType& type) {
	if (type != WebSocketMessageType::Text) return;

	Json::Value m;
	try {
		m = parseJSON(message);
	} catch (JSONCPP_STRING err) {
		wsConnPtr->send(errorMsg(err));
		return;
	}

	std::string msgType;
	try {
		msgType = m["type"].asString();
	} catch (Json::Exception err) {
		wsConnPtr->send(errorMsg(err.what()));
		return;
	}

	auto info = wsConnPtr->getContext<SocketInfo>();

	if (msgType == "uuid") {
			wsConnPtr->send(uuidMsg(info->getUuid()));
	} else if (msgType == "offer") {
		auto p = rooms[info->getRoom()].getParticipant(m["uuid"].asString());

		if (p.has_value()) {
			p.value()->send(offerMsg(info->getUuid(), m["offer"]));
		} else {
			wsConnPtr->send(errorMsg("no such uuid"));
		}
	} else if (msgType == "answer") {
		auto p = rooms[info->getRoom()].getParticipant(m["uuid"].asString());

		if (p.has_value()) {
			p.value()->send(answerMsg(info->getUuid(), m["answer"]));
		} else {
			wsConnPtr->send(errorMsg("no such uuid"));
		}
	} else if (msgType == "ice") {
		auto p = rooms[info->getRoom()].getParticipant(m["uuid"].asString());

		if (p.has_value()) {
			p.value()->send(iceMsg(info->getUuid(), m["candidate"]));
		} else {
			wsConnPtr->send(errorMsg("no such uuid"));
		}
	}
#ifdef USE_LUA_PLUGINS
	else if (msgType == "plugin") {
		participants_mutex.lock();
		pluginManager.passMsg(m["id"].asInt(), info->getUuid(), m["msg"].asString());
		participants_mutex.unlock();
	}
#endif
	else {
			wsConnPtr->send(errorMsg("no server implementation for message type: " + msgType));
	}
}

void StreamSock::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr) {
	int room_id = 0;

	try {
		room_id = std::stoi(req->getParameter("id"));
	} catch (std::exception) {
		wsConnPtr->send(errorMsg("invalid room id, defaulting to 0"));
	}

	auto info = std::make_shared<SocketInfo>(drogon::utils::getUuid(), room_id);
	wsConnPtr->setContext(info);

	rooms[room_id].join(info->getUuid(), wsConnPtr);
}

void StreamSock::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr) {
	auto info = wsConnPtr->getContext<SocketInfo>();
	std::string uuid = info->getUuid();

	rooms[info->getRoom()].leave(uuid);

#ifdef USE_LUA_PLUGINS
	pluginManager.onLeave(uuid);
#endif
}

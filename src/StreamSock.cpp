#include "StreamSock.h"
#include "Util.h"
#include "Messages.h"

enum class msgType {
	uuid,
	offer,
	answer,
	ice,
#ifdef USE_LUA_PLUGINS
	plugin,
#endif
};

const std::unordered_map<std::string, msgType> typelookup = {
	{"uuid", msgType::uuid},
	{"offer", msgType::offer},
	{"answer", msgType::answer},
	{"ice", msgType::ice},
#ifdef USE_LUA_PLUGINS
	{"plugin", msgType::plugin},
#endif
};

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
			wsConnPtr->send(uuidMsg(info->getUuid()));
			break;
		case msgType::offer:
			{
				auto p = rooms[info->getRoom()].getParticipant(m["uuid"].asString());

				if (p.has_value()) {
					p.value()->send(offerMsg(info->getUuid(), m["offer"]));
				} else {
					wsConnPtr->send(errorMsg("no such uuid"));
				}
			}

			break;
		case msgType::answer:
			{
				auto p = rooms[info->getRoom()].getParticipant(m["uuid"].asString());

				if (p.has_value()) {
					p.value()->send(answerMsg(info->getUuid(), m["answer"]));
				} else {
					wsConnPtr->send(errorMsg("no such uuid"));
				}
			}

			break;
		case msgType::ice:
			{
				auto p = rooms[info->getRoom()].getParticipant(m["uuid"].asString());

				if (p.has_value()) {
					p.value()->send(iceMsg(info->getUuid(), m["candidate"]));
				} else {
					wsConnPtr->send(errorMsg("no such uuid"));
				}
			}

			break;
#ifdef USE_LUA_PLUGINS
		case msgType::plugin:
			participants_mutex.lock();
			pluginManager.passMsg(m["id"].asInt(), info->getUuid(), m["msg"].asString());
			participants_mutex.unlock();
			break;
#endif
		default:
			wsConnPtr->send(debugMsg("no server implementation for message type: " + typestring));
	}
}

void StreamSock::handleNewConnection(const HttpRequestPtr &req,const WebSocketConnectionPtr& wsConnPtr) {
	auto info = std::make_shared<SocketInfo>(drogon::utils::getUuid(), 0);
	wsConnPtr->setContext(info);

	const std::string join_msg = joinMsg(info->getUuid());

	Json::Value to_send(Json::arrayValue);

	for (auto i : rooms[0].allParticipants()) {
		// inform all participants of new participant
		i->send(join_msg);

		// sync state with new connection
		// this is a json object because I may want to attach additional information in the future like a username
		auto p = i->getContext<SocketInfo>();
		Json::Value part_json;
		part_json["uuid"] = p->getUuid();
		to_send.append(part_json);
	}

	// TODO: investigate potential race condition here

	rooms[0].join(info->getUuid(), wsConnPtr);

#ifdef USE_LUA_PLUGINS
	pluginManager.onJoin(info->getUuid());
#endif

	wsConnPtr->send(syncMsg(to_send));
}

void StreamSock::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr) {
	std::string uuid = wsConnPtr->getContext<SocketInfo>()->getUuid();
	const std::string leave_msg = leaveMsg(uuid);

	rooms[0].leave(uuid);

	// TODO: investigate potential race condition here

	for (auto i : rooms[0].allParticipants()) {
		i->send(leave_msg);
	}

#ifdef USE_LUA_PLUGINS
	pluginManager.onLeave(uuid);
#endif
}

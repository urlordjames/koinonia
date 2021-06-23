#include "StreamSock.h"
#include "SocketInfo.h"
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

std::unordered_set<WebSocketConnectionPtr> StreamSock::participants;
std::mutex participants_mutex;

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
			// TODO: maybe use std::map instead
			participants_mutex.lock();

			for (auto i : participants) {
				auto p = i->getContext<SocketInfo>();
				if (p->getUuid() == m["uuid"].asString()) {
					i->send(offerMsg(info->getUuid(), m["offer"]));
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
				if (p->getUuid() == m["uuid"].asString()) {
					i->send(answerMsg(info->getUuid(), m["answer"]));
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
				if (p->getUuid() == m["uuid"].asString()) {
					i->send(iceMsg(info->getUuid(), m["candidate"]));
					participants_mutex.unlock();
					return;
				}
			}

			participants_mutex.unlock();

			wsConnPtr->send(errorMsg("no such uuid"));
			break;
#ifdef USE_LUA_PLUGINS
		case msgType::plugin:
			pluginManager.passMsg(m["id"].asInt(), info->getUuid(), m["msg"].asString());
			break;
#endif
		default:
			wsConnPtr->send(debugMsg("no server implementation for message type: " + typestring));
	}
}

void StreamSock::handleNewConnection(const HttpRequestPtr &req,const WebSocketConnectionPtr& wsConnPtr) {
	auto info = std::make_shared<SocketInfo>();
	info->setUuid(drogon::utils::getUuid());
	wsConnPtr->setContext(info);

	Json::Value to_send(Json::arrayValue);

	participants_mutex.lock();

	for (auto i : participants) {
		// inform all participants of new participant
		i->send(joinMsg(info->getUuid()));

		// sync state with new connection
		// this is a json object because I may want to attach additional information in the future like a username
		auto p = i->getContext<SocketInfo>();
		Json::Value part_json;
		part_json["uuid"] = p->getUuid();
		to_send.append(part_json);
	}

	participants.insert(wsConnPtr);

#ifdef USE_LUA_PLUGINS
	pluginManager.onJoin(info->getUuid());
#endif

	participants_mutex.unlock();

	wsConnPtr->send(syncMsg(to_send));
}

void StreamSock::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr) {
	std::string uuid = wsConnPtr->getContext<SocketInfo>()->getUuid();

	participants_mutex.lock();
	participants.erase(wsConnPtr);

	for (auto i : participants) {
		i->send(leaveMsg(uuid));
	}

#ifdef USE_LUA_PLUGINS
	pluginManager.onLeave(uuid);
#endif

	participants_mutex.unlock();
}

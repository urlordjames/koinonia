#include "Messages.h"
#include "Util.h"

// human readable info

std::string errorMsg(std::string message) {
	Json::Value error;
	error["type"] = "error";
	error["msg"] = message;
	return stringify(error);
}

std::string debugMsg(std::string message) {
	Json::Value debug;
	debug["type"] = "debug";
	debug["msg"] = message;
	return stringify(debug);
}

// participant sync

std::string uuidMsg(const std::string &uuid) {
	Json::Value msg;
	msg["type"] = "uuid";
	msg["uuid"] = uuid;
	return stringify(msg);
}

std::string syncMsg(const Json::Value &peers) {
	Json::Value sync;
	sync["type"] = "sync";
	sync["peers"] = peers;
	return stringify(sync);
}

std::string joinMsg(const std::string &uuid) {
	Json::Value msg;
	msg["type"] = "join";
	msg["uuid"] = uuid;
	return stringify(msg);
}

std::string leaveMsg(const std::string &uuid) {
	Json::Value msg;
	msg["type"] = "leave";
	msg["uuid"] = uuid;
	return stringify(msg);
}

// signaling

std::string offerMsg(const std::string &uuid, const Json::Value &offer) {
	Json::Value msg;
	msg["type"] = "offer";
	msg["uuid"] = uuid;
	msg["offer"] = offer;
	return stringify(msg);
}

std::string answerMsg(const std::string &uuid, const Json::Value &answer) {
	Json::Value msg;
	msg["type"] = "answer";
	msg["uuid"] = uuid;
	msg["answer"] = answer;
	return stringify(msg);
}

std::string iceMsg(const std::string &uuid, const Json::Value &candidate) {
	Json::Value msg;
	msg["type"] = "ice";
	msg["uuid"] = uuid;
	msg["candidate"] = candidate;
	return stringify(msg);
}

// plugins

std::string pluginMsg(int id, const std::string &message) {
	Json::Value msg;
	msg["type"] = "plugin";
	msg["id"] = id;
	msg["msg"] = message;
	return stringify(msg);
}

std::string moduleMsg(const std::string &script) {
	Json::Value msg;
	msg["type"] = "module";
	msg["script"] = script;
	return stringify(msg);
}

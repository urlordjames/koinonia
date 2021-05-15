#include "Messages.h"
#include "Util.h"

// human readable info

std::string errorMsg(std::string message) {
	Json::Value error;
	error["type"] = "error";
	error["message"] = message;
	return stringify(error);
}

std::string debugMsg(std::string message) {
	Json::Value debug;
	debug["type"] = "debug";
	debug["message"] = message;
	return stringify(debug);
}

// participant sync

std::string uuidMsg(std::string &uuid) {
	Json::Value msg;
	msg["type"] = "uuid";
	msg["uuid"] = uuid;
	return stringify(msg);
}

std::string syncMsg(Json::Value &peers) {
	Json::Value sync;
	sync["type"] = "sync";
	sync["peers"] = peers;
	return stringify(sync);
}

std::string joinMsg(std::string &uuid) {
	Json::Value msg;
	msg["type"] = "join";
	msg["uuid"] = uuid;
	return stringify(msg);
}

std::string leaveMsg(std::string &uuid) {
	Json::Value msg;
	msg["type"] = "leave";
	msg["uuid"] = uuid;
	return stringify(msg);
}

// signaling

std::string offerMsg(std::string &uuid, Json::Value &offer) {
	Json::Value msg;
	msg["type"] = "offer";
	msg["uuid"] = uuid;
	msg["offer"] = offer;
	return stringify(msg);
}

std::string answerMsg(std::string &uuid, Json::Value &answer) {
	Json::Value msg;
	msg["type"] = "answer";
	msg["uuid"] = uuid;
	msg["answer"] = answer;
	return stringify(msg);
}

std::string iceMsg(std::string &uuid, Json::Value &candidate) {
	Json::Value msg;
	msg["type"] = "ice";
	msg["uuid"] = uuid;
	msg["candidate"] = candidate;
	return stringify(msg);
}

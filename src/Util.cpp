#include "Util.h"

Json::Value parseJSON(std::string *str) {
	Json::Value value;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	JSONCPP_STRING err;
	bool success = reader->parse(str->c_str(), str->c_str() + (str->length() * sizeof(char)), &value, &err);
	delete reader;
	if (success) {
		return value;
	} else {
		throw err;
	}
}

std::string stringify(Json::Value *v) {
	Json::StreamWriterBuilder builder;
	return Json::writeString(builder, *v);
}

std::string errorMsg(std::string message) {
	Json::Value error;
	error["type"] = "error";
	error["message"] = message;
	return stringify(&error);
}

std::string debugMsg(std::string message) {
	Json::Value debug;
	debug["type"] = "debug";
	debug["message"] = message;
	return stringify(&debug);
}

std::string syncMsg(Json::Value *peers) {
	Json::Value sync;
	sync["type"] = "sync";
	sync["peers"] = *peers;
	return stringify(&sync);
}

std::string answerMsg(Json::Value *answer) {
	Json::Value msg;
	msg["type"] = "answer";
	msg["message"] = *answer;
	return stringify(&msg);
}

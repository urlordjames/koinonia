#include "Util.h"

Json::CharReaderBuilder builder;
Json::CharReader *reader = builder.newCharReader();

Json::Value parseJSON(std::string &str) {
	Json::Value value;
	JSONCPP_STRING err;
	const char *c_str = str.c_str();
	bool success = reader->parse(c_str, c_str + str.length(), &value, &err);
	if (success) {
		return value;
	} else {
		throw err;
	}
}

std::string stringify(Json::Value &v) {
	Json::StreamWriterBuilder builder;
	return Json::writeString(builder, v);
}

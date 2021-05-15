#include "Util.h"

Json::Value parseJSON(std::string *str) {
	Json::Value value;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	JSONCPP_STRING err;
	bool success = reader->parse(str->c_str(), str->c_str() + str->length(), &value, &err);
	delete reader;
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

#include "Util.h"

std::unique_ptr<Json::Value> parseJSON(std::string *str) {
  Json::Value *value = new Json::Value();
  Json::CharReaderBuilder builder;
  Json::CharReader *reader = builder.newCharReader();
  JSONCPP_STRING err;
  bool success = reader->parse(str->c_str(), str->c_str() + (str->length() * sizeof(char)), value, &err);
  delete reader;
  if (success) {
    return std::unique_ptr<Json::Value>(value);
  } else {
    delete value;
    throw err;
  }
}

std::unique_ptr<std::string> stringify(Json::Value *v) {
  Json::StreamWriterBuilder builder;
  return std::make_unique<std::string>(Json::writeString(builder, *v));
}

std::unique_ptr<std::string> errorMsg(std::string message) {
  Json::Value error;
  error["type"] = "error";
  error["message"] = message;
  return stringify(&error);
}

std::unique_ptr<std::string> debugMsg(std::string message) {
  Json::Value debug;
  debug["type"] = "debug";
  debug["message"] = message;
  return stringify(&debug);
}

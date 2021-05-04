#include <json/json.h>
#include <memory>

Json::Value parseJSON(std::string *str);

std::string stringify(Json::Value &v);

// message builders
std::string errorMsg(std::string message);
std::string debugMsg(std::string message);
std::string uuidMsg(std::string &uuid);
std::string syncMsg(Json::Value &peers);
std::string offerMsg(std::string &uuid, Json::Value &offer);
std::string answerMsg(std::string &uuid, Json::Value &answer);
std::string iceMsg(std::string uuid, std::string candidate);

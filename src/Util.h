#include <json/json.h>

Json::Value parseJSON(std::string *str);

std::string stringify(Json::Value &v);

// message builders
std::string errorMsg(std::string message);
std::string debugMsg(std::string message);
std::string uuidMsg(std::string &uuid);
std::string syncMsg(Json::Value &peers);
std::string offerMsg(std::string &uuid, Json::Value &offer);
std::string answerMsg(std::string &uuid, Json::Value &answer);
std::string iceMsg(std::string &uuid, Json::Value &candidate);

// may soon deprecate syncMsg()
std::string joinMsg(std::string &uuid);
std::string leaveMsg(std::string &uuid);

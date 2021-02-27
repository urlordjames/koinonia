#include <json/json.h>
#include <memory>

Json::Value parseJSON(std::string *str);

std::string stringify(Json::Value *v);

// message builders
std::string errorMsg(std::string message);
std::string debugMsg(std::string message);

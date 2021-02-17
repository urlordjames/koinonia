#include <json/json.h>
#include <memory>

std::unique_ptr<Json::Value> parseJSON(std::string *str);

std::unique_ptr<std::string> stringify(Json::Value *v);

// message builders
std::unique_ptr<std::string> errorMsg(std::string message);
std::unique_ptr<std::string> debugMsg(std::string message);

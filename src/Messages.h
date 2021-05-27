#include <json/json.h>

// for passing human readable info to client
std::string errorMsg(std::string message);
std::string debugMsg(std::string message);

// for participant sync
std::string uuidMsg(const std::string &uuid);
std::string syncMsg(const Json::Value &peers);
std::string joinMsg(const std::string &uuid);
std::string leaveMsg(const std::string &uuid);

// for WebRTC signaling
std::string offerMsg(const std::string &uuid, const Json::Value &offer);
std::string answerMsg(const std::string &uuid, const Json::Value &answer);
std::string iceMsg(const std::string &uuid, const Json::Value &candidate);

// for plugins to pass custom messages
std::string pluginMsg(const Json::Value &content);

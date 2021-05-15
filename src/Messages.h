#include <json/json.h>

// for passing human readable info to client
std::string errorMsg(std::string message);
std::string debugMsg(std::string message);

// for participant sync
std::string uuidMsg(std::string &uuid);
std::string syncMsg(Json::Value &peers);
std::string joinMsg(std::string &uuid);
std::string leaveMsg(std::string &uuid);

// for WebRTC signaling
std::string offerMsg(std::string &uuid, Json::Value &offer);
std::string answerMsg(std::string &uuid, Json::Value &answer);
std::string iceMsg(std::string &uuid, Json::Value &candidate);

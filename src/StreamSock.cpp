#include "StreamSock.h"
#include "SocketInfo.h"
#include <iostream>
#include "Util.h"

enum types{
  sdp,
  stream
};

const std::unordered_map<std::string, types> typelookup = {{"sdp", sdp}, {"stream", stream}};

void StreamSock::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string&& message, const WebSocketMessageType& type) {
  if (type != WebSocketMessageType::Text) return;
  try {
    std::unique_ptr<Json::Value> m = parseJSON(&message);
    std::string typestring = (*m)["type"].asString();
    try {
      const types type = typelookup.at(typestring);
      switch (type) {
        case sdp:
          wsConnPtr->send(debugMsg("your sdp is: " + (*m)["sdp"].asString()));
        break;
	default:
	  wsConnPtr->send(debugMsg("no server implementation for message type: " + typestring));
      }
    } catch (std::out_of_range err) {
      wsConnPtr->send(errorMsg("no such message type: " + typestring));
    }
  } catch (JSONCPP_STRING err) {
    wsConnPtr->send(errorMsg(err));
    return;
  }
}

void StreamSock::handleNewConnection(const HttpRequestPtr &req,const WebSocketConnectionPtr& wsConnPtr) {
  wsConnPtr->send(debugMsg("hello!"));
}

void StreamSock::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr) {
  // stuff
}

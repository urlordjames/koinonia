#include <drogon/WebSocketController.h>

using namespace drogon;
class StreamSock: public drogon::WebSocketController<StreamSock> {
public:
	virtual void handleNewMessage(const WebSocketConnectionPtr&,
	                              std::string&&,
	                              const WebSocketMessageType&) override;
	virtual void handleNewConnection(const HttpRequestPtr &,
	                                 const WebSocketConnectionPtr&)override;
	virtual void handleConnectionClosed(const WebSocketConnectionPtr&)override;

	static std::unordered_set<WebSocketConnectionPtr> participants;
	
	WS_PATH_LIST_BEGIN
		WS_PATH_ADD("/stream");
	WS_PATH_LIST_END
};

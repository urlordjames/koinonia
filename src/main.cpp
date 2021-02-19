#include <drogon/HttpAppFramework.h>

int main() {
	drogon::app().enableBrotli(true);
	drogon::app().setDocumentRoot("static");
	drogon::app().addListener("0.0.0.0", 8080).run();
}

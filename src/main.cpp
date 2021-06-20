#include <drogon/HttpAppFramework.h>
#include <filesystem>

int main() {
	drogon::app().setDocumentRoot("static");
	drogon::app().setUploadPath(std::filesystem::temp_directory_path());
	drogon::app().addListener("0.0.0.0", 8080).run();
}

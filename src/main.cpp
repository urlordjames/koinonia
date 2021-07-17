#include <drogon/HttpAppFramework.h>
#include <filesystem>

int main() {
	drogon::app().setDocumentRoot("static");
	std::filesystem::path upload_path = std::filesystem::temp_directory_path() / "koinonia_uploads";
	std::filesystem::create_directories(upload_path);
	drogon::app().setUploadPath(upload_path.string());
	drogon::app().addListener("0.0.0.0", 8080).run();
}

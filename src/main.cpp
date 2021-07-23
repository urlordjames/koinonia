#include <drogon/HttpAppFramework.h>
#include <filesystem>

int main() {
	drogon::app().setDocumentRoot(STATIC_ROOT);

	std::filesystem::path upload_path = std::filesystem::temp_directory_path() / "koinonia_uploads";
	// explicit string conversion required, doesn't compile on Windows otherwise
	drogon::app().setUploadPath(upload_path.string());

	drogon::app().addListener("0.0.0.0", 8080).run();
}

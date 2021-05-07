#include <drogon/HttpAppFramework.h>
#include <iostream>

int main() {
	drogon::app().enableBrotli(true);
	drogon::app().setDocumentRoot("static");

	char *cert_file = getenv("CERT_FILE");
	char *key_file = getenv("KEY_FILE");

	if (cert_file != nullptr && key_file != nullptr) {
		bool use_ssl = drogon::app().supportSSL();

		if (!use_ssl) {
			std::cout << "WARNING: required SSL libraries are NOT installed, falling back to http" << std::endl;
		}

		drogon::app().addListener("0.0.0.0", 8080, use_ssl, cert_file, key_file).run();
	} else {
		drogon::app().addListener("0.0.0.0", 8080).run();
	}
}

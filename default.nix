let pkgs = import (fetchTarball https://github.com/urlordjames/nixpkgs/archive/drogon_options.tar.gz) {};
in pkgs.stdenv.mkDerivation rec {
	pname = "koinonia";
	version = "0.1.0";

	src = ./.;

	nativeBuildInputs = with pkgs; [
		cmake
	];

	buildInputs = with pkgs; [
		(drogon.override {
			brotliSupport = false;
			c-aresSupport = false;
			sqliteSupport = false;
		})
		lua
	];

	installPhase = "mkdir -p $out/bin && mv koinonia $out/bin";
}

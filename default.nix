let pkgs = import <unstable> {};
in pkgs.stdenv.mkDerivation rec {
	pname = "koinonia";
	version = "0.1.0";

	src = ./.;

	nativeBuildInputs = with pkgs; [
		cmake
	];

	buildInputs = with pkgs; [
		(drogon.override {
			sqliteSupport = false;
		})
		lua
	];

	installPhase = "mkdir -p $out/bin && mv koinonia $out/bin";
}

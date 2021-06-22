let pkgs = import (fetchTarball https://github.com/NixOS/nixpkgs/archive/nixpkgs-unstable.tar.gz) {};
in pkgs.stdenv.mkDerivation rec {
	pname = "koinonia";
	version = "0.1.0";

	src = ./.;

	nativeBuildInputs = with pkgs; [
		cmake
	];

	buildInputs = with pkgs; [
		(drogon.override {
			# TODO: uncomment when this hits unstable
			#sqliteSupport = false;
		})
		lua
	];

	installPhase = "mkdir -p $out/bin && mv koinonia $out/bin";
}

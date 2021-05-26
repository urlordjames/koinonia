{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
	nativeBuildInputs = with pkgs; [
		cmake
		ninja
	];

	buildInputs = with pkgs; [
		openssl
		jsoncpp
		libuuid
		zlib
		lua
	];
}

{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  buildInputs = with pkgs; [
    cmake
    ninja
    openssl
    jsoncpp
    libuuid
    brotli
    zlib
  ];
}

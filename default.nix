let pkgs = import <unstable> {};
in pkgs.stdenv.mkDerivation rec {
	pname = "koinonia";
	version = "0.1.0";

	src = ./.;

	nativeBuildInputs = with pkgs; [
		cmake
	];

	buildInputs = with pkgs; [
		drogon
		lua
	];

	preConfigure = ''
		export cmakeFlags="$cmakeFlags -DSTATIC_ROOT=$out/extras/static"
	'';
}

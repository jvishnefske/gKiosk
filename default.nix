{ lib, stdenv, cmake, ninja, pkg-config
, gtk3, webkitgtk_4_1, boost
}:

stdenv.mkDerivation {
  pname = "gkiosk";
  version = "1.0.0";

  src = lib.cleanSource ./.;

  nativeBuildInputs = [ cmake ninja pkg-config ];
  buildInputs = [ gtk3 webkitgtk_4_1 boost ];

  cmakeFlags = [ "-GNinja" ];

  installPhase = ''
    install -D -m 0755 gKiosk $out/bin/gkiosk
  '';

  meta = with lib; {
    description = "Lightweight fullscreen kiosk web browser";
    license = licenses.mit;
    platforms = platforms.linux;
  };
}

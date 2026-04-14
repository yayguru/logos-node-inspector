{
  description = "Read-only Logos Basecamp package for inspecting a single Logos VPS";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { nixpkgs, ... }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs systems;
    in {
      packages = forAllSystems (system:
        let
          pkgs = import nixpkgs {
            inherit system;
          };
        in {
          default = pkgs.stdenv.mkDerivation {
            pname = "logos-node-inspector";
            version = "0.1.0";
            src = ./.;

            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
              pkgs.pkg-config
              pkgs.qt6.wrapQtAppsHook
            ];

            buildInputs = [
              pkgs.qt6.qtbase
              pkgs.qt6.qtdeclarative
            ];

            configurePhase = ''
              runHook preConfigure
              cmake -S . -B build -GNinja
              runHook postConfigure
            '';

            buildPhase = ''
              runHook preBuild
              cmake --build build
              runHook postBuild
            '';

            installPhase = ''
              runHook preInstall
              cmake --install build --prefix "$out"
              runHook postInstall
            '';
          };
        });
    };
}

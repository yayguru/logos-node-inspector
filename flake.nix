{
  description = "Read-only Logos Basecamp module for inspecting a single Logos VPS";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    logos-module-builder.url = "github:logos-co/logos-module-builder";
  };

  outputs = { self, nixpkgs, flake-utils, logos-module-builder }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        packages.default = logos-module-builder.lib.mkLogosModule {
          inherit pkgs system;
          src = ./.;
          configFile = ./module.yaml;
        };
      });
}


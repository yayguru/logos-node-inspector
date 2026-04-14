{
  description = "Read-only Logos Basecamp module for inspecting a single Logos VPS";

  inputs = {
    logos-module-builder.url = "github:logos-co/logos-module-builder";
  };

  outputs = { self, logos-module-builder, ... }:
    logos-module-builder.lib.mkLogosModule {
      src = ./.;
      configFile = ./module.yaml;
    };
}

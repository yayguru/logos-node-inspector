# Logos Node Inspector

`logos-node-inspector` is a read-only Basecamp module for inspecting a single Linux VPS that runs `logos-node` and `nwaku`.

This first milestone is intentionally narrow:

- one VPS
- key-based SSH only
- read-only collection
- Logos-first health views for `/network/info` and `/cryptarchia/info`
- operational guidance without remote mutation

## What It Collects

The module uses the system `ssh` client to execute a curated, read-only shell bundle on the remote VPS and parses the result into typed state for the UI.

Collected signals:

- host basics: hostname, kernel, uptime
- host health: root disk usage, memory usage, Docker disk summary
- service health: `docker ps`, `docker inspect` for `logos-node` and `nwaku`
- Logos runtime: `http://127.0.0.1:<apiPort>/network/info`
- Logos runtime: `http://127.0.0.1:<apiPort>/cryptarchia/info`
- recent service logs for `logos-node` and `nwaku`

## Project Layout

- [module.yaml](C:\Users\gadin\Desktop\temporal\logosnodehostinger\logos-node-inspector\module.yaml): module scaffold for `logos-module-builder`
- [flake.nix](C:\Users\gadin\Desktop\temporal\logosnodehostinger\logos-node-inspector\flake.nix): Nix entry point for Logos module packaging
- [metadata.json](C:\Users\gadin\Desktop\temporal\logosnodehostinger\logos-node-inspector\metadata.json): Basecamp package metadata
- [CMakeLists.txt](C:\Users\gadin\Desktop\temporal\logosnodehostinger\logos-node-inspector\CMakeLists.txt): Qt/QML build and tests
- [src](C:\Users\gadin\Desktop\temporal\logosnodehostinger\logos-node-inspector\src): parser, classifier, SSH collector, controller
- [qml](C:\Users\gadin\Desktop\temporal\logosnodehostinger\logos-node-inspector\qml): Basecamp-facing UI
- [scripts/inspect-logos-vps.sh](C:\Users\gadin\Desktop\temporal\logosnodehostinger\logos-node-inspector\scripts\inspect-logos-vps.sh): manual collector reference
- [tests](C:\Users\gadin\Desktop\temporal\logosnodehostinger\logos-node-inspector\tests): fixture-driven parser and classification coverage
- [docs/ubuntu-basecamp-checklist.md](C:\Users\gadin\Desktop\temporal\logosnodehostinger\logos-node-inspector\docs\ubuntu-basecamp-checklist.md): Ubuntu 24.04 build/load checklist for Basecamp validation

## Development Notes

This workspace is Windows-based and does not include the Logos/Basecamp Linux runtime, Qt toolchain, or Nix stack needed to build a `.lgx` artifact here end-to-end. The code is structured for Linux/macOS Basecamp packaging, but Linux-side validation still needs to happen in a Logos-compatible environment.

The parser and classification logic are kept independent from the UI layer so they can be tested with captured fixture output.

## Linux Validation

Use the Ubuntu guide in [docs/ubuntu-basecamp-checklist.md](C:\Users\gadin\Desktop\temporal\logosnodehostinger\logos-node-inspector\docs\ubuntu-basecamp-checklist.md) for the first full Basecamp run.

The current UI is a workbench:

- fixed left rail navigation
- one active pane at a time
- manual refresh only
- lazy log pane
- Linux-adjacent dark palette with restrained status accents

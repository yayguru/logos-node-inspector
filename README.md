# Logos Node Inspector

Logos Node Inspector is a Basecamp module for operators running a single Logos VPS. It gives you a clean, read-only workbench for checking `logos-node`, `nwaku`, host pressure, sync state, and recent logs without touching the remote machine.

## Why It Exists

Running a Logos node usually means switching between SSH, Docker, logs, and local API probes just to answer simple operational questions:

- Is `logos-node` healthy?
- Is the node syncing or stuck?
- Is `nwaku` alive?
- Is the VPS running out of disk or memory?
- What should I check next when something degrades?

This module pulls those checks into one Basecamp-native interface and keeps the interaction deliberately read-only.

## Current Scope

Version one is intentionally narrow:

- one VPS
- key-based SSH only
- read-only collection
- manual refresh only
- Logos-aware health checks for `/network/info` and `/cryptarchia/info`
- no restart, prune, cleanup, or remote file mutation actions

## What The Module Shows

- `Overview`: node health, sync mode, peers, disk, containers, last refresh
- `Logos`: peer id, mode, height, slot, lib, tip, endpoint reachability
- `Services`: `logos-node` and `nwaku` container state, health, restart policy, ports, image
- `Host`: kernel, uptime, disk usage, memory usage, Docker disk summary
- `Recommendations`: short operator guidance with copyable read-only commands
- `Logs`: capped recent logs for `logos-node` and `nwaku`
- `Connection`: single-profile SSH configuration for the target VPS

## How It Works

The module runs locally inside Basecamp and uses the system `ssh` client to execute a curated read-only command bundle on the VPS. It then parses the result into typed state for the UI.

Collected signals include:

- host basics: hostname, kernel, uptime
- host health: root disk usage, memory usage, Docker disk summary
- service health: `docker ps`, `docker inspect` for `logos-node` and `nwaku`
- Logos runtime: `http://127.0.0.1:<apiPort>/network/info`
- Logos runtime: `http://127.0.0.1:<apiPort>/cryptarchia/info`
- recent logs for `logos-node` and `nwaku`

## Interface Direction

The UI is designed as a quiet operator workbench rather than a glossy dashboard:

- dark neutral surfaces
- no gradients
- no pure black or pure white
- muted status colors instead of generic neon health states
- strict invisible grid and dense alignment
- one active pane at a time
- lazy loading for heavier views such as logs

## Project Structure

- [`module.yaml`](./module.yaml): module scaffold for Logos packaging
- [`flake.nix`](./flake.nix): Nix entry point for building the package
- [`metadata.json`](./metadata.json): Basecamp package metadata
- [`CMakeLists.txt`](./CMakeLists.txt): Qt/QML build and test entry point
- [`src`](./src): parser, classifier, SSH collector, controller
- [`qml`](./qml): Basecamp-facing UI
- [`tests`](./tests): fixture-driven parser and classification coverage
- [`scripts/inspect-logos-vps.sh`](./scripts/inspect-logos-vps.sh): manual reference collector
- [`docs/ubuntu-basecamp-checklist.md`](./docs/ubuntu-basecamp-checklist.md): Ubuntu 24.04 build and validation guide

## Build And Validation

Ubuntu 24.04 is the first-class validation target for this project.

The expected flow is:

1. Install Basecamp on Ubuntu.
2. Clone this repository.
3. Build the module with `nix build`.
4. Load the generated package into Basecamp.
5. Configure the VPS SSH profile.
6. Run a manual refresh and validate the rendered state against the real node.

Use the full checklist here:

- [`docs/ubuntu-basecamp-checklist.md`](./docs/ubuntu-basecamp-checklist.md)

## Development Status

The current repository includes:

- the module scaffold
- the SSH collector and parser pipeline
- typed status classification
- the workbench-style QML interface
- Windows-side GitHub bootstrap helpers for publishing the repo

The remaining milestone is full Linux packaging and Basecamp validation in a real Ubuntu environment.

## License

MIT. See [`LICENSE`](./LICENSE).

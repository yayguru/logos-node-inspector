# Ubuntu 24.04 Basecamp Checklist

This checklist is the first-run validation path for `logos-node-inspector` on Ubuntu 24.04 using the latest Logos Basecamp pre-release.

## 1. Install host prerequisites

```bash
sudo add-apt-repository universe
sudo apt update
sudo apt install -y curl git libfuse2t64
```

## 2. Install Nix with flakes enabled

Use the current Determinate or official Nix installer, then confirm:

```bash
nix --version
nix flake show
```

If flakes are not enabled yet, enable them before continuing.

## 3. Download the latest Basecamp Linux AppImage

Use the current pre-release page:

- [Logos Basecamp releases](https://github.com/logos-co/logos-basecamp/releases)

Download the latest Linux AppImage asset to your Ubuntu machine.

## 4. Make the AppImage executable and launch it once

```bash
chmod +x ~/Downloads/*.AppImage
~/Downloads/*.AppImage
```

Basecamp should start successfully on Ubuntu 24.04 after `libfuse2t64` is installed.

## 5. Build the module from the repo root

From the module root:

```bash
cd /path/to/logos-node-inspector
nix build
```

## 6. Verify the build output

```bash
ls -la result
find result -maxdepth 3 \( -name '*.lgx' -o -name '*.zip' -o -name '*.tar.gz' \)
```

Identify the module artifact that Basecamp can import.

## 7. Load the local module package into Basecamp

Open Basecamp and use the package-manager local install flow to import the built artifact.

During the first Ubuntu validation, record the literal menu labels you actually use and replace the placeholders below with the exact Basecamp wording:

1. `[package-manager entry label]`
2. `[local install/import label]`
3. `[confirm/install label]`

Do not leave these placeholders in the checklist after the first real Linux validation.

## 8. Configure the SSH profile

Use key-based SSH only.

Fill these fields in the `Connection` pane:

- `Profile name`
- `Host`
- `SSH user`
- `Identity file`
- `Port`
- `logos container`
- `nwaku container`
- `API port`
- optional `Use sudo for Docker`

Save the profile before refreshing.

## 9. Run one manual refresh

Use the `Refresh` button once and confirm the module does not poll in the background.

## 10. Validate the snapshot

Confirm all of the following:

- `Overview` opens by default when the profile is configured
- `Connection` is the default only for an empty first-run profile
- `logos-node` service status renders
- `nwaku` service status renders
- `/network/info` renders
- `/cryptarchia/info` renders
- root disk state matches the VPS
- Docker disk summary matches the VPS
- recommendations only offer read-only commands
- `Logs` only instantiate when the `Logs` pane is opened

## 11. Confirm read-only behavior

The module must not:

- restart containers
- prune Docker
- modify files on the VPS
- create keys
- write remote config

The remote interaction should remain SSH command collection plus local `curl` calls on the VPS only.

## 12. Capture the final validation notes

Record:

- Ubuntu version used
- Basecamp pre-release tag used
- exact AppImage filename
- exact Basecamp menu labels used for local package install
- built artifact filename
- whether the real VPS matched the expected Logos health and sync state

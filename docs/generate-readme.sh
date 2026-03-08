#!/bin/bash
# generate-readme.sh - Generates README.md with build instructions,
# architecture details, and desktop screenshot.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
OUTPUT="${PROJECT_DIR}/docs/RELEASE_NOTES.md"
README="${PROJECT_DIR}/README.md"

# Check for screenshot
SCREENSHOT_LINE=""
if [ -f "${PROJECT_DIR}/docs/desktop-screenshot.png" ]; then
    SCREENSHOT_LINE="
## Desktop Screenshot

![NakumiOS Desktop](docs/desktop-screenshot.png)
"
fi

cat > "${README}" << 'READMEEOF'
# NakumiOS v1.0

A modern, lightweight Linux desktop operating system built on Debian Testing (Trixie).

## Features

- **Custom Wayland Compositor** (nakumi-wm) - Built with wlroots using the wlr_scene graph API
- **Qt6 Desktop Environment** - Panel, Launcher, and desktop shell using native Wayland protocols
- **Core Applications** - Terminal, File Manager, Text Editor, Settings
- **Modern Design** - Dark theme with purple accents, minimalist UI
- **VM Compatible** - Automatic software rendering fallback for VirtualBox/VMware/QEMU
- **PipeWire Audio** - Modern audio stack with WirePlumber session management
- **Auto-login** - Boots directly to desktop via greetd
READMEEOF

if [ -n "$SCREENSHOT_LINE" ]; then
    echo "$SCREENSHOT_LINE" >> "${README}"
fi

cat >> "${README}" << 'READMEEOF'

## Architecture

```
┌─────────────────────────────────────────────────┐
│                NakumiOS Desktop                  │
├──────────┬──────────┬──────────┬────────────────┤
│ Panel    │ Launcher │ Terminal │  Apps (Qt6)     │
├──────────┴──────────┴──────────┴────────────────┤
│            nakumi-wm (wlroots compositor)        │
├─────────────────────────────────────────────────┤
│  Wayland │ PipeWire │  D-Bus  │ NetworkManager  │
├─────────────────────────────────────────────────┤
│              Linux Kernel (Debian Testing)       │
└─────────────────────────────────────────────────┘
```

## Design Tokens

| Token       | Value     |
|-------------|-----------|
| Background  | `#1A1A24` |
| Accent      | `#6C5CE7` |
| Text        | `#E2E8F0` |
| Radius      | `12px`    |

## Build from Source

### Prerequisites

- Debian Testing (Trixie) or Ubuntu 24.04+
- `live-build`, `meson`, `cmake`, `ninja-build`
- Qt6 development packages
- wlroots development packages

### Build the ISO

```bash
# Build all components
make build-all

# Build the live ISO (requires root for live-build)
make iso
```

### Test in QEMU

```bash
# Launch in QEMU
make run

# Automated testing with screenshots
make test-automation
```

### Run Unit Tests

```bash
make test
```

### Create a GitHub Release

```bash
make publish
```

## Component Overview

| Component       | Language | Build System | Description                        |
|-----------------|----------|--------------|------------------------------------|
| nakumi-wm       | C        | Meson        | Wayland compositor (wlroots)       |
| nakumi-panel    | C++20    | CMake        | Bottom dock panel (layer-shell)    |
| nakumi-launcher | C++20    | CMake        | Fullscreen application launcher    |
| nakumi-term     | C++20    | CMake        | Terminal emulator (forkpty+poll)    |
| nakumi-files    | C++20    | CMake        | File manager (UDisks2 D-Bus)       |
| nakumi-edit     | C++20    | CMake        | Text editor (syntax highlighting)  |
| nakumi-settings | C++20    | CMake        | System settings (nmcli, wpctl)     |

## License

MIT License - See LICENSE for details.
READMEEOF

echo "README.md generated."

# Also generate release notes
cat > "${OUTPUT}" << 'RELEASEEOF'
## NakumiOS v1.0.0 Release

### What's New

- Initial release of NakumiOS
- Custom wlroots-based Wayland compositor with wlr_scene API
- Qt6 Desktop Environment with layer-shell panel and application launcher
- Core applications: Terminal, File Manager, Text Editor, Settings
- Debian Testing (Trixie) base with live-build
- PipeWire audio with WirePlumber
- Automatic software rendering fallback for VMs
- Auto-login via greetd

### System Requirements

- x86_64 processor
- 2 GB RAM minimum (4 GB recommended)
- 4 GB disk space
- GPU with DRM/KMS support (or runs in software rendering mode)

### Checksums

See the accompanying `.sha256` file for ISO integrity verification.
RELEASEEOF

echo "RELEASE_NOTES.md generated."

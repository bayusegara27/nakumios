# NakumiOS

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

A bespoke, highly aesthetic Linux-based operating system built with Qt6/QML featuring a custom Wayland compositor and modern desktop shell.

![NakumiOS Banner](resources/banner.png)

## ✨ Features

- **Custom Wayland Compositor** - Built with QtWayland Compositor API for fluid window management
- **Modern Desktop Shell** - QML-based shell with blur effects, animations, and system integration
- **Deep Dark Theme** - Electric purple accent (#6C5CE7) with glass-morphism design
- **Native Applications** - Settings, File Manager, Terminal built with Qt6
- **Rolling Release** - Based on Arch Linux via archiso for latest packages

## 🎨 Design Language

| Element | Value |
|---------|-------|
| Background | `#0F0F14` |
| Surface | `#1A1A24` @ 60% opacity |
| Accent | `#6C5CE7` (Electric Purple) |
| Border | `1px solid #2D2D3B` |
| Radius | `12px` |
| Typography | Inter / Roboto |

## 🏗️ Architecture

```
nakumios/
├── src/
│   ├── common/          # Shared libraries (DesignTokens, SystemInfo)
│   ├── compositor/      # QtWayland compositor
│   ├── shell/           # Desktop shell (panels, dock, launcher)
│   ├── services/        # D-Bus services (session, notifications)
│   └── apps/            # Core applications
│       ├── settings/    # System settings
│       ├── terminal/    # Terminal emulator
│       └── filemanager/ # File manager
├── archiso/             # ISO build configuration
├── resources/           # Themes, fonts, icons
└── scripts/             # Build scripts
```

## 🛠️ Technology Stack

| Component | Technology |
|-----------|------------|
| Base OS | Arch Linux (archiso) |
| Display Server | Wayland |
| Compositor | QtWayland Compositor (C++) |
| UI/UX | Qt6 + QML (Qt Quick) |
| IPC | D-Bus (Qt D-Bus) |
| Init System | systemd |
| Audio | PipeWire |
| Testing | QEMU/KVM |

## 📦 Building

### Prerequisites

```bash
# Arch Linux / Manjaro
sudo pacman -S base-devel cmake ninja qt6-base qt6-declarative \
    qt6-wayland qt6-quickcontrols2 archiso

# Ubuntu / Debian (Qt6 packages may differ)
sudo apt install build-essential cmake ninja-build \
    qt6-base-dev qt6-declarative-dev qt6-wayland-dev
```

### Build Commands

```bash
# Build all components
./scripts/build.sh --build

# Install to staging directory
./scripts/build.sh --install

# Create bootable ISO (requires root)
sudo ./scripts/build.sh --iso

# Test in QEMU
./scripts/build.sh --qemu

# Clean build artifacts
./scripts/build.sh --clean
```

### CMake Direct Build

```bash
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
```

## 🧪 Testing

### QEMU Testing

```bash
# Quick test (requires existing ISO)
./scripts/build.sh --qemu

# Manual QEMU command
qemu-system-x86_64 \
    -enable-kvm \
    -m 4G \
    -smp 4 \
    -cdrom output/nakumios-*.iso \
    -vga virtio \
    -display gtk,gl=on
```

## 📁 Project Components

### Compositor (`src/compositor/`)
- `compositor.cpp` - Main Wayland compositor implementation
- `windowmanager.cpp` - Window management and z-ordering

### Shell (`src/shell/`)
- `Main.qml` - Root shell component
- `panels/TopPanel.qml` - Top panel with activities, clock, system tray
- `panels/Dock.qml` - Application dock
- `launcher/AppLauncher.qml` - Full-screen app launcher

### Core Applications

| Application | Description |
|-------------|-------------|
| **Settings** | System configuration (appearance, display, sound) |
| **Terminal** | PTY-based terminal emulator |
| **Files** | File manager with bookmarks and navigation |

## 🔧 Configuration

### Design Tokens

All design constants are defined in `src/common/designtokens.h`:

```cpp
// Colors
QColor backgroundBase() const { return QColor(0x0F, 0x0F, 0x14); }
QColor accentPrimary() const { return QColor(0x6C, 0x5C, 0xE7); }

// Dimensions
int borderRadius() const { return 12; }
int panelHeight() const { return 48; }

// Effects
qreal panelOpacity() const { return 0.6; }
int blurRadius() const { return 32; }
```

### QML Usage

```qml
import NakumiOS 1.0

Rectangle {
    color: DesignTokens.backgroundBase
    radius: DesignTokens.borderRadius
    
    Text {
        color: DesignTokens.textPrimary
        font.family: DesignTokens.fontFamily
    }
}
```

## 📄 License

MIT License - see [LICENSE](LICENSE) for details.

## 🤝 Contributing

Contributions are welcome! Please read our contributing guidelines before submitting PRs.

## 📞 Support

- Issues: [GitHub Issues](https://github.com/nakumios/nakumios/issues)
- Discussions: [GitHub Discussions](https://github.com/nakumios/nakumios/discussions)

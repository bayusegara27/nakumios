# ==============================================================================
# NakumiOS Master Makefile
# ==============================================================================
# Orchestrates build, test, and release of NakumiOS live ISO.
#
# Targets:
#   make iso              - Build the live ISO using Debian live-build
#   make run              - Launch the ISO in QEMU for testing
#   make build-wm         - Build the Wayland compositor (nakumi-wm)
#   make build-de         - Build the desktop environment (panel + launcher)
#   make build-apps       - Build the core applications
#   make build-all        - Build all components
#   make test             - Run unit tests (QtTest)
#   make test-automation  - Run QEMU integration tests with screenshots
#   make publish          - Create a GitHub release with the ISO
#   make clean            - Clean all build artifacts
# ==============================================================================

SHELL := /bin/bash
.PHONY: iso run build-wm build-de build-apps build-all test test-automation \
        publish clean install-deps

# Configuration
ISO_NAME     := nakumios-1.0-amd64
ISO_FILE     := live-build-config/$(ISO_NAME).iso
CHROOT_BIN   := live-build-config/config/includes.chroot/usr/bin
CHROOT_APPS  := live-build-config/config/includes.chroot/usr/share/applications
QEMU_MEM     := 2048
QEMU_CPUS    := 2
VERSION      := 1.0.0

# ==============================================================================
# Build Components
# ==============================================================================

build-wm:
	@echo "==> Building nakumi-wm (Wayland compositor)..."
	cd nakumi-wm && meson setup builddir --prefix=/usr && ninja -C builddir
	@echo "==> nakumi-wm build complete."

build-panel:
	@echo "==> Building nakumi-panel..."
	cd nakumi-de/panel && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr && \
		cmake --build build
	@echo "==> nakumi-panel build complete."

build-launcher:
	@echo "==> Building nakumi-launcher..."
	cd nakumi-de/launcher && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr && \
		cmake --build build
	@echo "==> nakumi-launcher build complete."

build-de: build-panel build-launcher

build-term:
	@echo "==> Building nakumi-term..."
	cd nakumi-apps/nakumi-term && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr && \
		cmake --build build
	@echo "==> nakumi-term build complete."

build-files:
	@echo "==> Building nakumi-files..."
	cd nakumi-apps/nakumi-files && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr && \
		cmake --build build
	@echo "==> nakumi-files build complete."

build-edit:
	@echo "==> Building nakumi-edit..."
	cd nakumi-apps/nakumi-edit && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr && \
		cmake --build build
	@echo "==> nakumi-edit build complete."

build-settings:
	@echo "==> Building nakumi-settings..."
	cd nakumi-apps/nakumi-settings && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr && \
		cmake --build build
	@echo "==> nakumi-settings build complete."

build-apps: build-term build-files build-edit build-settings

build-all: build-wm build-de build-apps

# ==============================================================================
# Install into chroot overlay
# ==============================================================================

install-chroot: build-all
	@echo "==> Installing binaries into live-build chroot overlay..."
	mkdir -p $(CHROOT_BIN)
	mkdir -p $(CHROOT_APPS)
	# Compositor
	cp nakumi-wm/builddir/nakumi-wm $(CHROOT_BIN)/
	# Desktop Environment
	cp nakumi-de/panel/build/nakumi-panel $(CHROOT_BIN)/
	cp nakumi-de/launcher/build/nakumi-launcher $(CHROOT_BIN)/
	# Applications
	cp nakumi-apps/nakumi-term/build/nakumi-term $(CHROOT_BIN)/
	cp nakumi-apps/nakumi-files/build/nakumi-files $(CHROOT_BIN)/
	cp nakumi-apps/nakumi-edit/build/nakumi-edit $(CHROOT_BIN)/
	cp nakumi-apps/nakumi-settings/build/nakumi-settings $(CHROOT_BIN)/
	# Ensure start-nakumi is executable
	chmod +x $(CHROOT_BIN)/start-nakumi
	@echo "==> Chroot overlay installation complete."

# ==============================================================================
# ISO Build (Debian live-build)
# ==============================================================================

iso: install-chroot
	@echo "==> Building NakumiOS live ISO..."
	cd live-build-config && lb clean --purge 2>/dev/null || true
	cd live-build-config && lb config
	cd live-build-config && sudo lb build
	@echo "==> ISO built: $(ISO_FILE)"

# ==============================================================================
# QEMU Testing
# ==============================================================================

run:
	@echo "==> Launching NakumiOS in QEMU..."
	@if [ ! -f "$(ISO_FILE)" ]; then \
		echo "ERROR: ISO not found at $(ISO_FILE). Run 'make iso' first."; \
		exit 1; \
	fi
	qemu-system-x86_64 \
		-enable-kvm \
		-m $(QEMU_MEM) \
		-smp $(QEMU_CPUS) \
		-cdrom $(ISO_FILE) \
		-boot d \
		-vga virtio \
		-display gtk \
		-device virtio-net-pci,netdev=net0 \
		-netdev user,id=net0 \
		-device intel-hda \
		-device hda-duplex \
		-usb \
		-device usb-tablet

test-automation:
	@echo "==> Running automated UI tests in QEMU..."
	@if [ ! -f "$(ISO_FILE)" ]; then \
		echo "ERROR: ISO not found at $(ISO_FILE). Run 'make iso' first."; \
		exit 1; \
	fi
	mkdir -p docs
	qemu-system-x86_64 \
		-enable-kvm \
		-m $(QEMU_MEM) \
		-smp $(QEMU_CPUS) \
		-cdrom $(ISO_FILE) \
		-boot d \
		-vga virtio \
		-display none \
		-device virtio-net-pci,netdev=net0 \
		-netdev user,id=net0 \
		-virtfs local,path=./docs,mount_tag=host0,security_model=passthrough,id=host0 \
		-serial mon:stdio \
		-device intel-hda \
		-device hda-duplex \
		-usb \
		-device usb-tablet
	@echo "==> Automated tests complete. Check docs/ for screenshots."

# ==============================================================================
# Unit Tests
# ==============================================================================

test:
	@echo "==> Running unit tests..."
	cd tests && cmake -B build && cmake --build build && ctest --test-dir build --output-on-failure
	@echo "==> All tests passed."

# ==============================================================================
# GitHub Release
# ==============================================================================

publish:
	@echo "==> Publishing NakumiOS release..."
	@if [ ! -f "$(ISO_FILE)" ]; then \
		echo "ERROR: ISO not found. Run 'make iso' first."; \
		exit 1; \
	fi
	# Generate SHA256 checksum
	sha256sum $(ISO_FILE) > $(ISO_FILE).sha256
	# Generate documentation
	./docs/generate-readme.sh
	# Create GitHub release
	gh release create v$(VERSION) \
		$(ISO_FILE) \
		$(ISO_FILE).sha256 \
		--title "NakumiOS v$(VERSION)" \
		--notes-file docs/RELEASE_NOTES.md \
		--draft
	@echo "==> Release v$(VERSION) created as draft."

# ==============================================================================
# Clean
# ==============================================================================

clean:
	@echo "==> Cleaning all build artifacts..."
	rm -rf nakumi-wm/builddir
	rm -rf nakumi-de/panel/build
	rm -rf nakumi-de/launcher/build
	rm -rf nakumi-apps/nakumi-term/build
	rm -rf nakumi-apps/nakumi-files/build
	rm -rf nakumi-apps/nakumi-edit/build
	rm -rf nakumi-apps/nakumi-settings/build
	rm -rf tests/build
	cd live-build-config && lb clean --purge 2>/dev/null || true
	@echo "==> Clean complete."

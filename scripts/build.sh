#!/bin/bash
# NakumiOS Build Script
# Builds all components and creates an ISO image

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_ROOT}/build"
ISO_OUTPUT="${PROJECT_ROOT}/output"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check dependencies
check_dependencies() {
    log_info "Checking build dependencies..."
    
    local deps=("cmake" "ninja" "git")
    local missing=()
    
    for dep in "${deps[@]}"; do
        if ! command -v "$dep" &> /dev/null; then
            missing+=("$dep")
        fi
    done
    
    if [ ${#missing[@]} -ne 0 ]; then
        log_error "Missing dependencies: ${missing[*]}"
        log_info "Please install them using your package manager."
        exit 1
    fi
    
    log_success "All dependencies found."
}

# Build NakumiOS components
build_components() {
    log_info "Building NakumiOS components..."
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    cmake "$PROJECT_ROOT" \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DBUILD_TESTS=OFF
    
    ninja
    
    log_success "Build completed successfully."
}

# Install to staging area
install_to_staging() {
    log_info "Installing to staging area..."
    
    local staging_dir="${BUILD_DIR}/staging"
    mkdir -p "$staging_dir"
    
    cd "$BUILD_DIR"
    DESTDIR="$staging_dir" ninja install
    
    log_success "Installation completed."
}

# Build ISO using archiso (requires root)
build_iso() {
    log_info "Building ISO image..."
    
    if [ "$EUID" -ne 0 ]; then
        log_warning "ISO building requires root privileges."
        log_info "Please run: sudo $0 --iso"
        return 1
    fi
    
    local archiso_work="${BUILD_DIR}/archiso-work"
    local profile_dir="${PROJECT_ROOT}/archiso"
    
    mkdir -p "$archiso_work" "$ISO_OUTPUT"
    
    # Copy built binaries to airootfs
    local airootfs="${profile_dir}/airootfs"
    cp -r "${BUILD_DIR}/staging/"* "$airootfs/"
    
    # Build ISO
    mkarchiso -v -w "$archiso_work" -o "$ISO_OUTPUT" "$profile_dir"
    
    log_success "ISO created in $ISO_OUTPUT"
}

# Run in QEMU for testing
run_qemu() {
    log_info "Starting QEMU..."
    
    local iso_file=$(find "$ISO_OUTPUT" -name "nakumios*.iso" -type f | head -1)
    
    if [ -z "$iso_file" ]; then
        log_error "No ISO file found in $ISO_OUTPUT"
        log_info "Please build the ISO first: $0 --iso"
        exit 1
    fi
    
    log_info "Booting: $iso_file"
    
    qemu-system-x86_64 \
        -enable-kvm \
        -m 4G \
        -smp 4 \
        -cpu host \
        -cdrom "$iso_file" \
        -boot d \
        -vga virtio \
        -display gtk,gl=on \
        -device virtio-tablet-pci \
        -device virtio-keyboard-pci \
        -device intel-hda \
        -device hda-duplex \
        -nic user,model=virtio-net-pci
}

# Clean build artifacts
clean() {
    log_info "Cleaning build artifacts..."
    rm -rf "$BUILD_DIR"
    rm -rf "$ISO_OUTPUT"
    log_success "Clean completed."
}

# Print usage
usage() {
    echo "NakumiOS Build Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --build       Build all NakumiOS components"
    echo "  --install     Install to staging directory"
    echo "  --iso         Build bootable ISO (requires root)"
    echo "  --qemu        Run ISO in QEMU for testing"
    echo "  --clean       Clean build artifacts"
    echo "  --all         Build everything and create ISO"
    echo "  -h, --help    Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 --build           # Build components"
    echo "  $0 --qemu            # Test in QEMU"
    echo "  sudo $0 --iso        # Create ISO"
}

# Main
main() {
    if [ $# -eq 0 ]; then
        usage
        exit 0
    fi
    
    while [ $# -gt 0 ]; do
        case "$1" in
            --build)
                check_dependencies
                build_components
                ;;
            --install)
                install_to_staging
                ;;
            --iso)
                build_iso
                ;;
            --qemu)
                run_qemu
                ;;
            --clean)
                clean
                ;;
            --all)
                check_dependencies
                build_components
                install_to_staging
                build_iso
                ;;
            -h|--help)
                usage
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                usage
                exit 1
                ;;
        esac
        shift
    done
}

main "$@"

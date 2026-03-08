project(
    'nakumi-wm',
    'c',
    version: '1.0.0',
    license: 'MIT',
    default_options: [
        'c_std=c11',
        'warning_level=2',
        'werror=false',
    ],
)

cc = meson.get_compiler('c')

# Core dependencies
wlroots = dependency('wlroots-0.18', version: '>= 0.18.0')
wayland_server = dependency('wayland-server')
wayland_protocols = dependency('wayland-protocols')
xkbcommon = dependency('xkbcommon')
pixman = dependency('pixman-1')

# Wayland protocol scanner
wayland_scanner = find_program('wayland-scanner')
wl_protocol_dir = wayland_protocols.get_variable('pkgdatadir')

# Generate xdg-shell protocol
xdg_shell_xml = wl_protocol_dir / 'stable/xdg-shell/xdg-shell.xml'
xdg_shell_c = custom_target(
    'xdg-shell-protocol',
    input: xdg_shell_xml,
    output: 'xdg-shell-protocol.c',
    command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
)
xdg_shell_h = custom_target(
    'xdg-shell-protocol-header',
    input: xdg_shell_xml,
    output: 'xdg-shell-protocol.h',
    command: [wayland_scanner, 'server-header', '@INPUT@', '@OUTPUT@'],
)

# Generate wlr-layer-shell protocol
layer_shell_xml = 'protocols/wlr-layer-shell-unstable-v1.xml'
layer_shell_c = custom_target(
    'wlr-layer-shell-protocol',
    input: layer_shell_xml,
    output: 'wlr-layer-shell-unstable-v1-protocol.c',
    command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
)
layer_shell_h = custom_target(
    'wlr-layer-shell-protocol-header',
    input: layer_shell_xml,
    output: 'wlr-layer-shell-unstable-v1-protocol.h',
    command: [wayland_scanner, 'server-header', '@INPUT@', '@OUTPUT@'],
)

nakumi_wm_sources = files(
    'src/main.c',
)

executable(
    'nakumi-wm',
    sources: [
        nakumi_wm_sources,
        xdg_shell_c,
        xdg_shell_h,
        layer_shell_c,
        layer_shell_h,
    ],
    dependencies: [
        wlroots,
        wayland_server,
        xkbcommon,
        pixman,
    ],
    install: true,
)

# NakumiOS archiso profile
# Based on Arch Linux releng profile

# Image settings
iso_name="nakumios"
iso_label="NAKUMIOS_$(date +%Y%m)"
iso_publisher="NakumiOS Project <https://nakumios.org>"
iso_application="NakumiOS Live/Install ISO"
iso_version="$(date +%Y.%m.%d)"
install_dir="nakumios"
buildmodes=('iso')
bootmodes=('bios.syslinux.mbr' 'bios.syslinux.eltorito' 'uefi-ia32.grub.esp' 'uefi-x64.grub.esp')
arch="x86_64"
pacman_conf="pacman.conf"
airootfs_image_type="squashfs"
airootfs_image_tool_options=('-comp' 'xz' '-Xbcj' 'x86' '-b' '1M' '-Xdict-size' '1M')
file_permissions=(
  ["/etc/shadow"]="0:0:400"
  ["/root"]="0:0:750"
  ["/usr/local/bin/nakumi-session"]="0:0:755"
)

#!/usr/bin/env bash
# devkitPro's own servers (apt/pkg.devkitpro.org) return Cloudflare 403 from the owner's network,
# so the toolchain is taken from the official Docker image instead:
#   (Windows, Git Bash)  docker create --name dkp_export devkitpro/devkita64:20260219
#                        docker cp dkp_export:/opt/devkitpro - > /d/Kirikiroid2-Switch/.cache/devkitpro.tar
#                        docker rm dkp_export
#   (WSL, as root)       bash ${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/src/platforms/switch/tools/install_devkitpro_from_docker.sh
set -euo pipefail
TAR=${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/.cache/devkitpro.tar
rm -rf /opt/devkitpro
tar -xf "$TAR" -C /opt
cat > /etc/profile.d/devkit-env.sh <<'EOF'
export DEVKITPRO=/opt/devkitpro
export PATH=/opt/devkitpro/tools/bin:/opt/devkitpro/devkitA64/bin:$PATH
EOF
echo "installed: $(du -sh /opt/devkitpro | cut -f1)"

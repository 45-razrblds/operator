#!/bin/bash
set -e

# Farben und Icons
BLUE='\033[0;34m'
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Nerd Font Icons
ICON_INFO=ℹ️
ICON_BUILD=🔨
ICON_DOCKER=🐳
ICON_ISO=💿
ICON_SUCCESS=✅
ICON_ERROR=❌
ICON_GIT=📦
ICON_TIME=⏰

echo -e "${BLUE}${ICON_INFO} Operator OS Build System${NC}"
echo -e "${BLUE}${ICON_TIME} Generiere Build-Informationen...${NC}"

# Generate build information
YEAR=$(date +%Y)
MONTH=$(date +%m)
DAY=$(date +%d)
TIME=$(date +%H%M)
GIT_HASH=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
BUILD_NUMBER="${YEAR}${MONTH}${DAY}.${TIME}.${GIT_HASH}"
VERSION="1.8"
BUILD_DATE="$(date '+%Y-%m-%d %H:%M:%S')"

echo -e "${GREEN}${ICON_GIT} Build: ${BUILD_NUMBER}${NC}"
echo -e "${GREEN}${ICON_TIME} Datum: ${BUILD_DATE}${NC}"

# Update version.h with build information
echo -e "${BLUE}${ICON_INFO} Aktualisiere version.h...${NC}"
cat > kernel/version.h << EOL
#ifndef VERSION_H
#define VERSION_H

#define OS_VERSION "${VERSION}"
#define BUILD_NUMBER "${BUILD_NUMBER}"
#define BUILD_DATE "${BUILD_DATE}"

const char* get_os_version(void) { return OS_VERSION; }
const char* get_build_number(void) { return BUILD_NUMBER; }
const char* get_build_date(void) { return BUILD_DATE; }

#endif // VERSION_H
EOL

# Build kernel and boot objects on host
echo -e "${YELLOW}${ICON_BUILD} Baue Kernel...${NC}"
make clean
make os.bin

# Use Docker to build the ISO with GRUB
DOCKER_IMAGE=operator-build-env

echo -e "${BLUE}${ICON_DOCKER} Baue Docker Image...${NC}"
docker build -t $DOCKER_IMAGE .

echo -e "${BLUE}${ICON_ISO} Erstelle ISO Image...${NC}"
docker run --rm -v "$PWD":/workspace $DOCKER_IMAGE bash -c '
  set -e
  mkdir -p isodir/boot/grub
  cp os.bin isodir/boot/os.bin
  echo -e "set timeout=1\nmenuentry \"Operator OS v${VERSION} (Build ${BUILD_NUMBER})\" {\n multiboot /boot/os.bin\n}" > isodir/boot/grub/grub.cfg
  grub-mkrescue -o os.iso isodir
'

echo -e "${GREEN}${ICON_SUCCESS} ISO Image erfolgreich erstellt: os.iso${NC}"
echo -e "${BLUE}${ICON_INFO} Starten mit: qemu-system-i386 -cdrom os.iso -netdev user,id=net0 -device rtl8139,netdev=net0${NC}"

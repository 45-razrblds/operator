#!/bin/bash
set -e

# Generate build information
YEAR=$(date +%Y)
MONTH=$(date +%m)
DAY=$(date +%d)
TIME=$(date +%H%M)
GIT_HASH=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
BUILD_NUMBER="${YEAR}${MONTH}${DAY}.${TIME}.${GIT_HASH}"
VERSION="1.8"
BUILD_DATE="$(date '+%Y-%m-%d %H:%M:%S')"

# Update version.h with build information
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
make clean
make os.bin

# Use Docker to build the ISO with GRUB
DOCKER_IMAGE=operator-build-env

docker build -t $DOCKER_IMAGE .
docker run --rm -v "$PWD":/workspace $DOCKER_IMAGE bash -c '
  set -e
  mkdir -p isodir/boot/grub
  cp os.bin isodir/boot/os.bin
  echo -e "set timeout=1\nmenuentry \"Operator OS v${VERSION} (Build ${BUILD_NUMBER})\" {\n multiboot /boot/os.bin\n}" > isodir/boot/grub/grub.cfg
  grub-mkrescue -o os.iso isodir
'

echo "ISO image built as os.iso. You can now run: qemu-system-i386 -cdrom os.iso"

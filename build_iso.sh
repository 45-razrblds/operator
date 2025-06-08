#!/bin/bash
set -e

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
  echo -e "set timeout=1\nmenuentry \"My OS\" {\n multiboot /boot/os.bin\n}" > isodir/boot/grub/grub.cfg
  grub-mkrescue -o os.iso isodir
'

echo "ISO image built as os.iso. You can now run: qemu-system-i386 -cdrom os.iso"
